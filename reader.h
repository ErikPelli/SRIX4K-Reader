#ifndef READER_H
#define READER_H

#include <stdint.h>
#include <nfc/nfc.h>
#include "error.h"

#define MAX_DEVICE_COUNT  8
#define MAX_TARGET_COUNT  1

/**
 * Single SRIX block.
 */
typedef struct SrixBlock {
    uint8_t block[SRIX_BLOCK_LENGTH];
} SrixBlock;

/**
 * Struct that represents a NFC Reader.
 */
typedef struct NfcReader {
    nfc_connstring libnfc_readers[MAX_DEVICE_COUNT];  /* readers connstring array */
    nfc_device *libnfc_reader;                        /* libnfc reader */
} NfcReader;

/**
 * Allocate a nfc reader and set its default values.
 * @return null if there is an error, else an nfc reader pointer
 */
NfcReader *NfcReaderNew();

/**
 * Close a nfc reader.
 * @param reader reader instance where reader is saved
 */
void NfcCloseReader(NfcReader *reader);

/**
 * Update available readers on NfcReader instance.
 * @param reader pointer to NfcReader
 * @return number of readers currently available and saved on instance
 */
size_t NfcUpdateReaders(NfcReader *reader);

/**
 * Get a description of a specific reader.
 * @param reader pointer to a NfcReader instance
 * @param selection index of reader to get
 * @return pointer to reader description string
 */
char *NfcGetReaderDescription(NfcReader *reader, int selection);

/**
 * Initialize an NFC Reader.
 * @param reader pointer to Reader struct
 * @param selection id of Reader to initialize
 * @return SrixError result
 */
SrixError NfcInitReader(NfcReader *reader, int selection);

/**
 * Get UID from Reader as raw byte array.
 * @param reader pointer to Reader struct
 * @param uid array where save the UID data
 * @return SrixError result
 */
SrixError NfcGetUid(NfcReader *reader, uint8_t uid[const static SRIX_UID_LENGTH]);

/**
 * Read a specified block from SRIX4K to block array.
 * @param reader nfc reader to send command
 * @param block array to save read block
 * @param blockNum block to read from SRIX
 * @return SrixError result
 */
SrixError NfcReadBlock(NfcReader *reader, SrixBlock *block, uint8_t blockNum);

/**
 * Write a specified block to SRIX4K.
 * @param reader nfc reader to send command
 * @param block array of data to write to block
 * @param blockNum block to write to SRIX
 * @return SrixError result
 */
SrixError NfcWriteBlock(NfcReader *reader, SrixBlock *block, uint8_t blockNum);

#endif /* READER_H */