#include "mem.hpp"

uintptr_t strtoptr(const char* str, char** endptr, int base){
    #ifdef _WIN32
        return strtoull(str, endptr, base);
    #else
        return strtoull(str, endptr, base);
    #endif
}

pid_t mem::GetPIDFromProcessName(const char* process_name) {
    char cmd[256];

    // Create a command to run in the shell, which will list all running processes and
    // filter out the process with the specified name using grep.
    snprintf(cmd, sizeof(cmd), "ps -e | grep -m1 '%s'", process_name);

    // Open a pipe to run the command and read its output.
    FILE* pipe = popen(cmd, "r");
    if (!pipe) {

        // If the pipe couldn't be opened, return an error code (-1).
        return 1;
    }

    // Read the first line of the output from the pipe.
    char line[256];
    fgets(line, sizeof(line), pipe);

    // Close the pipe.
    pclose(pipe);

    // If no output was produced by the command, return an error code (-1).
    if (strlen(line) == 0) {
        return 1;
    }

    // Parse the PID from the first token in the output line.
    char* pid_str = strtok(line, " ");
    if (!pid_str) {

        // If no token was found, return an error code (-1).
        return 1;
    }

    // Convert the PID string to an integer and return it.
    return atoi(pid_str);
}

// Thanks to obdr at GuidedHacking for the base ReadProcessMemory function.
// I, Dontna, added error handling, and added a bool return value for easier debugging.
// See their code here: https://guidedhacking.com/threads/linux-game-hacking-full-guide.16411/post-101577
bool mem::ReadProcessMemory(pid_t processID, void* src, void* dst, size_t size){
    /*
    processID  = target process id
    src  = address to read from on the target process
    dst  = address to write to on the caller process
    size = size of the buffer that will be read
    */

    struct iovec iosrc;
    struct iovec iodst;
    iodst.iov_base = dst;
    iodst.iov_len  = size;
    iosrc.iov_base = src;
    iosrc.iov_len  = size;

    ssize_t bytes_read = process_vm_readv(processID, &iodst, 1, &iosrc, 1, 0);
    if (bytes_read == -1) {
        std::cerr << "Error: Failed to read process memory at address " << src << " in process " << processID << ". " << strerror(errno) << "\n";
        return false;
    } else if ((size_t)bytes_read != size) {
        std::cerr << "Error: Only " << bytes_read << " bytes were read from address " << src << " in process " << processID << ", expected " << size << " bytes\n";
        return false;
    }
    return true;
}

// Thanks to obdr at GuidedHacking for the base WriteProcessMemory function.
// I, Dontna, added error handling, and added a bool return value for easier debugging.
// See their code here: https://guidedhacking.com/threads/linux-game-hacking-full-guide.16411/post-101577
bool mem::WriteProcessMemory(pid_t processID, void* dst, void* src, size_t size){
    /*
    processID  = target process id
    dst  = address to write to on the target process
    src  = address to read from on the caller process
    size = size of the buffer that will be read
    */

    struct iovec iosrc;
    struct iovec iodst;
    iosrc.iov_base = src;
    iosrc.iov_len  = size;
    iodst.iov_base = dst;
    iodst.iov_len  = size;

    if (process_vm_writev(processID, &iosrc, 1, &iodst, 1, 0) == -1) {
        std::cerr << "Error writing process memory.\n";
        return false;
    }
    return true;
}

/**
 * This function takes a base address and a vector of offsets, and uses them to traverse a 
 * pointer chain in a remote process, ultimately returning the memory address pointed to by 
 * the final pointer in the chain. 
 *
 * @param processID The process ID of the remote process.
 * @param baseAddress The starting address of the pointer chain.
 * @param offsets A vector of offsets to be added to each pointer in the chain.
 * @param debugText A boolean indicating whether to print debug information.
 *
 * @return The memory address pointed to by the final pointer in the chain.
 */

uintptr_t mem::GetAddressFromPointers(pid_t processID, const uintptr_t& baseAddress, const std::vector<int>& offsets, bool debugText) {
    void* pointerAddress;
    bool readSuccess;

    // Perform an initial read to get the value the base address is pointing to.
    if (debugText){
        printf("Number of offsets: %zu\n", offsets.size());
        printf("Base address before: %lx\n", baseAddress);
    }
    readSuccess = mem::ReadProcessMemory(processID, (void*)baseAddress, &pointerAddress, sizeof(pointerAddress));
    if (!readSuccess) {
        if (debugText){
            std::cerr << "Error: Failed to read " << sizeof(pointerAddress) << " bytes from address " << baseAddress << " in process " << processID << "\n";
        }
        return 0;
    }

    // Loop through all, but the final, pointers in the offets and read the address they point to.
    for (std::vector<int>::size_type i = 0; i < offsets.size() - 1; i++) {
        if (debugText) {
            printf("Base address after offset %lu: %lx\n", i, (uintptr_t)pointerAddress);
        }
        readSuccess = mem::ReadProcessMemory(processID, (void*)((uintptr_t)pointerAddress + offsets.at(i)), &pointerAddress, sizeof(pointerAddress));
        if (!readSuccess) {
            if (debugText){
                std::cerr << "Error: Failed to read " << sizeof(pointerAddress) << " bytes from address " << ((uintptr_t)pointerAddress + offsets.at(i)) << " in process " << processID << "\n";
            }
            return 0;
        }
    }

    // Add the last offset in the list to the pointer address to get the correct memory address for our value.
    return (uintptr_t)pointerAddress + offsets.at(offsets.size() - 1);
}


uintptr_t mem::GetModuleBaseAddress(pid_t processID, const std::string& module_name) {
    // Get the path of the 'maps' file for the given process
    std::stringstream maps_file_path;
    maps_file_path << "/proc/" << processID << "/maps";

    // Open the 'maps' file for the given process
    std::ifstream maps_file_fs(maps_file_path.str(), std::ios::binary);
    if (!maps_file_fs.is_open()) return 0; // Return 0 if file can't be opened

    // Read the content of the 'maps' file into a stringstream
    std::stringstream maps_file;
    maps_file << maps_file_fs.rdbuf();

    // Find the position of the module name in the 'maps' file
    size_t module_path_pos = maps_file.str().find(module_name);

    // Find the position of the base address of the module
    size_t base_address_pos = maps_file.str().rfind('\n', module_path_pos) + 1;
    size_t base_address_end = maps_file.str().find('-', base_address_pos);

    // Return 0 if the module name or base address couldn't be found
    if (base_address_pos == maps_file.str().npos || base_address_end == maps_file.str().npos) return 0;

    // Extract the base address string from the 'maps' file
    std::string base_address_str = maps_file.str().substr(base_address_pos, base_address_end - base_address_pos);

    // Convert the base address string to a uintptr_t
    std::uintptr_t base_address = std::stoull(base_address_str, nullptr, 16);

    // Close the 'maps' file stream
    maps_file_fs.close();

    // Return the base address of the module
    return base_address;
}

/**
 * @brief Replace bytes in the memory of a target process using ptrace.
 *
 * This function attaches to the specified process, reads the original data
 * at the given address, replaces the bytes according to the provided vector,
 * and writes the modified data back to the process memory. 
 * The number of bytes to replace, is corrolated to the size of the replacementBytes vector.
 *
 * @param processID Process ID of the target process.
 * @param destinationAddress Memory address in the target process to start replacing bytes.
 * @param newBytes Vector containing the new bytes to replace at the specified address.
 * @return True if the operation succeeds, false otherwise.
 */
bool mem::ReplaceBytes(pid_t processID, uintptr_t destinationAddress, const std::vector<uint8_t>& replacementBytes) {
    uintptr_t originalData;

    if (ptrace(PTRACE_ATTACH, processID, nullptr, nullptr) == -1) {
        perror("Error attaching to process with ptrace");
        return false;
    }

    waitpid(processID, nullptr, 0);

    // Read the original data at the specified address
    originalData = ptrace(PTRACE_PEEKTEXT, processID, (void*)destinationAddress, nullptr);
    
    if (static_cast<uintptr_t>(originalData) == static_cast<uintptr_t>(-1) && errno) {
        perror("Error reading the original bytes with ptrace");
        ptrace(PTRACE_DETACH, processID, nullptr, nullptr);
        return false;
    }

    // Modify the bytes, one at a time
    for (size_t i = 0; i < replacementBytes.size(); ++i) {
        uint8_t byte = replacementBytes[i];
        
        // Modify the specific byte in the originalData at position 'i' with the new byte value.
        // - Clear the original byte at position 'i' using bitwise AND with a mask.
        // - Set the cleared position with the new byte value using bitwise OR.
        //   The new byte is left-shifted to its correct position in the originalData.
        originalData = (originalData & ~(0xFFULL << (i * 8))) | (static_cast<uintptr_t>(byte) << ((i % (sizeof(uintptr_t) / sizeof(uint8_t))) * 8));
    }

    if (ptrace(PTRACE_POKETEXT, processID, (void*)destinationAddress, (void*)originalData) == -1) {
        perror("Error writing the modified bytes with ptrace");
        ptrace(PTRACE_DETACH, processID, nullptr, nullptr);
        return false;
    }

    if (ptrace(PTRACE_DETACH, processID, nullptr, nullptr) == -1) {
        perror("Error dettaching from process with ptrace.");
        return false;
    }

    return true;
}