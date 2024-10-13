#include <inttypes.h>
#include <stdio.h>

uint8_t count1(uint32_t x) {
    int out;
    __asm {
        mov edx, [x]
        mov al, 0
next:   cmp edx, 0
        je done
        mov cl, dl
        and cl, 1
        add al, cl
        shr edx, 1
        jmp next
done:
        mov out, al
    }
    return out;
}

int main() {
    uint32_t x = 0x5789ABCD;
    uint8_t cnt = count1(x);
    printf("Number of 1s in 0x%X: %hhu\n", x, cnt);
}
