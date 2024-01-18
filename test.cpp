#include <cstdio>

long long generateMask(int size) {
    long long mask = 0xFF;

    // Add "00" pairs to the mask based on the size
    mask <<= size * 8;

    return mask;
}

int main(){
    long long mask = generateMask(7);

    printf("Output: %x", mask);
}

