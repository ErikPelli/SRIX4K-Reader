#ifndef SRIX_FLAG_H
#define SRIX_FLAG_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Struct that represents the modified blocks in a SRIX tag
 */
typedef struct SrixFlag {
    uint32_t memory[4];
} SrixFlag;

#define SRIX_FLAG_INIT ((SrixFlag) {{0, 0, 0, 0}})

/**
 * Set the flag value of a specified block to true (modified).
 * @param flag pointer to a SrixFlag instance
 * @param block block to flag (0-127)
 */
void srixFlagAdd(SrixFlag *flag, uint8_t block);

/**
 * Get the flag value of a specified block.
 * @param flag pointer to a SrixFlag instance
 * @param block block to get (0-127)
 * @return boolean result
 */
bool srixFlagGet(SrixFlag *flag, uint8_t block);

#endif /* SRIX_FLAG_H */