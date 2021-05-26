#ifndef SRIX_H
#define SRIX_H

#include <stdint.h>
#include <stdlib.h>
#include "error.h"

typedef struct Srix Srix;

/**
 * Create a new Srix and set its default values.
 * @return null if there is an error, else a Srix struct pointer
 */
Srix *SrixNew();

/**
 * Delete a Srix and free its memory.
 * @param target Srix instance to delete
 */
void SrixDelete(Srix *target);

/**
 * Function that search for available NFC readers and return their number.
 * @param target pointer to Srix struct
 * @return number of readers found
 */
size_t NfcGetReadersCount(Srix *target);

/**
 * Function that return specified nfc reader description (connection string).
 * @param target pointer to Srix struct
 * @param reader index of reader (0 = first, 1 = second, ecc.)
 * @return connstring of reader at specified index
 */
char *NfcGetDescription(Srix *target, int reader);

/**
 * Initialize the Srix using Nfc.
 * @param target pointer to Srix struct
 * @param reader index of nfc reader to use
 * @return string error result
 */
const char *SrixNfcInit(Srix *target, int reader);

/**
 * Initialize the Srix using values in memory.
 * @param target pointer to Srix struct
 * @param eeprom pointer to EEPROM array to import
 * @param uid UID to import
 */
void SrixMemoryInit(Srix *target, uint32_t eeprom[const static SRIX4K_BLOCKS], uint64_t uid);

/**
 * Return UID of an initialized srix.
 * @param target pointer to Srix struct
 * @return uid uint64 value
 */
uint64_t SrixGetUid(Srix *target);

/**
 * Get pointer to a specified block.
 * @param target pointer to Srix struct
 * @param blockNum number of block to get
 * @return pointer to blockNum block
 */
uint32_t *SrixGetBlock(Srix *target, uint8_t blockNum);

/**
 * Modify manually a Srix block and add flag automatically.
 * @param target pointer to Srix struct
 * @param block value to write to blockNum
 * @param blockNum index of block to write
 */
void SrixModifyBlock(Srix *target, uint32_t block, uint8_t blockNum);

/**
 * Write all modified blocks of target to physical SRIX4K.
 * @param target pointer to Srix struct
 * @return numeric result, 0 = no error
 */
int SrixWriteBlocks(Srix *target);

#endif /* SRIX_H */