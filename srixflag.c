#include "srixflag.h"

void srixFlagAdd(SrixFlag flag[static 1], uint8_t block) {
    /*
     * Array:
     * 0 -> 0-31
     * 1-> 32-63
     * 2-> 64-95
     * 3-> 96-127
     *
     * Flag position bit between 0 and 31 in a single uint32_t
     * (array of 4 uint32_t).
     */
    if (block < 128) {
        flag->memory[block / 32] |= 1U << block % 32;
    }
}

bool srixFlagGet(SrixFlag flag[static 1], uint8_t block) {
    if (block < 128) {
        return flag->memory[block / 32] >> block % 32 & 1U;
    } else {
        return false;
    }
}