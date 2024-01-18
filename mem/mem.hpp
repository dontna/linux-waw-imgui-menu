#include <iostream>
#include <cstring>
#include <sstream>
#include <istream>
#include <fstream>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <sys/io.h>
#include <sys/uio.h>
#include <dlfcn.h>
#include <link.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdint>
#include <vector>

typedef struct _module_t
{
    std::string name;
    std::string path;
    void*       base;
    void*       end;
    uintptr_t   size;
    void*       handle; //this will not be used for now, only internally with dlopen
}module_t;

namespace mem {
    pid_t GetPIDFromProcessName(const char* process_name);
    bool ReadProcessMemory(pid_t pid, void* src, void* dst, size_t size);
    bool WriteProcessMemory(pid_t pid, void* dst, void* src, size_t size);
    bool ReplaceBytes(pid_t processID, uintptr_t destinationAddress, const std::vector<uint8_t>& replacementBytes);
    uintptr_t GetAddressFromPointers(pid_t processId, const uintptr_t& baseAddress, const std::vector<int>& offsets, bool debugText=false);
    uintptr_t GetModuleBaseAddress(pid_t pid, const std::string& module_name);
}