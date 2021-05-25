#include <stdlib.h>
#include <string.h>
#include "reader.h"

static const nfc_modulation nfc_ISO14443B = {
        .nmt = NMT_ISO14443B,
        .nbr = NBR_106,
};

static const nfc_modulation nfc_ISO14443B2SR = {
        .nmt = NMT_ISO14443B2SR,
        .nbr = NBR_106,
};

static nfc_context *libnfc_context = (void *) 0;

/**
 * Exit from libnfc context at the end of the execution.
 */
static void exitNfcContext() {
    nfc_exit(libnfc_context);
}

/**
 * Initialize libnfc context for this application.
 */
static void nfcContextInit() {
    if (!libnfc_context) {
        nfc_init(&libnfc_context);
        atexit(exitNfcContext);
    }
}

/**
 * Initialize a NFC device as reader.
 * @param reader pointer to a reader to initialize.
 * @param target string that represent a found reader.
 * @return SrixError instance, if there is an error it will include its description.
 */
static SrixError nfcReaderInit(NfcReader *reader, int target) {
    /* Open target reader */
    reader->libnfc_reader = nfc_open(libnfc_context, reader->libnfc_readers[target]);
    if (!reader->libnfc_reader) {
        return SRIX_ERROR(NFC_ERROR, "unable to open requested nfc reader");
    }

    /* NFC device is an initiator (a reader) */
    if (nfc_initiator_init(reader->libnfc_reader)) {
        nfc_close(reader->libnfc_reader);
        return SRIX_ERROR(NFC_ERROR, nfc_strerror(reader->libnfc_reader));
    }

    nfc_device_set_property_bool(reader->libnfc_reader, NP_INFINITE_SELECT, true);
    return SRIX_NO_ERROR;
}

/**
 * Search for a valid SRIX4K tag to initialize and do polling if it isn't available.
 * @param reader pointer to a NFC device.
 * @return SrixError instance, if there is an error it will include its description.
 */
static SrixError nfcSrix4kInit(NfcReader *reader) {
    /*
     * (libnfc) To read ISO14443B2SR you have to initiate first ISO14443B to configure PN532 internal registers.
     * https://github.com/nfc-tools/libnfc/issues/436#issuecomment-326686914
     */
    nfc_target tmpTarget[MAX_TARGET_COUNT];
    nfc_initiator_list_passive_targets(reader->libnfc_reader, nfc_ISO14443B, tmpTarget, MAX_TARGET_COUNT);

    /* NFC tag polling */
    if (nfc_initiator_select_passive_target(reader->libnfc_reader, nfc_ISO14443B2SR, (void *) 0, 0, tmpTarget) < 0) {
        nfc_close(reader->libnfc_reader);
        return SRIX_ERROR(NFC_ERROR, nfc_strerror(reader->libnfc_reader));
    } else {
        return SRIX_NO_ERROR;
    }
}

/**
 * Send bytes to the SRIX tag and save the response.
 * @param target NFC device pointer.
 * @param tx_data array of bytes to send.
 * @param tx_size number of bytes to send.
 * @param rx_data pointer to an array of bytes where save the response.
 * @param rx_size size of rx_data array.
 * @return NFC response length in bytes.
 */
static inline size_t nfcExchange(nfc_device *target, const uint8_t *restrict tx_data, const size_t tx_size,
                                 uint8_t *restrict rx_data, const size_t rx_size) {
    return nfc_initiator_transceive_bytes(target, tx_data, tx_size, rx_data, rx_size, 0);
}

NfcReader *NfcReaderNew() {
    /* Allocate struct */
    NfcReader *created = malloc(sizeof(NfcReader));
    if (!created) {
        return (void *) 0;
    }

    /* Initialize context and set nfc reader to null (avoid conflicts) */
    nfcContextInit();
    created->libnfc_reader = (void *) 0;

    /* Return struct pointer */
    return created;
}

void NfcCloseReader(NfcReader reader[static 1]) {
    nfc_close(reader->libnfc_reader);
}

size_t NfcUpdateReaders(NfcReader reader[static 1]) {
    /* Search for readers */
    return nfc_list_devices(libnfc_context, reader->libnfc_readers, MAX_DEVICE_COUNT);
}

char *NfcGetReaderDescription(NfcReader reader[static 1], int selection) {
    return reader->libnfc_readers[selection];
}

SrixError NfcInitReader(NfcReader reader[static 1], int selection) {
    /* Init Reader */
    SrixError error = nfcReaderInit(reader, selection);
    if (SRIX_IS_ERROR(error)) {
        return error;
    }

    /* Init SRIX */
    error = nfcSrix4kInit(reader);
    if (SRIX_IS_ERROR(error)) {
        return error;
    }

    return SRIX_NO_ERROR;
}

/* NFC commands */
#define SRIX_GET_UID      0x0B
#define SRIX_READ_BLOCK   0x08
#define SRIX_WRITE_BLOCK  0x09

SrixError NfcGetUid(NfcReader reader[static 1], uint8_t uid[const static SRIX_UID_LENGTH]) {
    /* Send command (length = 1) and check length */
    if (nfcExchange(reader->libnfc_reader, (const uint8_t[]) {SRIX_GET_UID}, 1, uid, SRIX_UID_LENGTH) !=
        SRIX_UID_LENGTH) {
        return SRIX_ERROR(NFC_ERROR, "invalid UID length");
    }

    return SRIX_NO_ERROR;
}


SrixError NfcReadBlock(NfcReader reader[static 1], SrixBlock block[static 1], const uint8_t blockNum) {
    /* Read while read block length is different than expected */
    do {
        if (nfc_initiator_target_is_present(reader->libnfc_reader, (void *) 0) < 0) {
            SrixError error = nfcSrix4kInit(reader);
            if (SRIX_IS_ERROR(error)) {
                return error;
            }
        }
    } while (nfcExchange(reader->libnfc_reader, (const uint8_t[]) {SRIX_READ_BLOCK, blockNum}, 2,
                         (uint8_t *) block, SRIX_BLOCK_LENGTH) != SRIX_BLOCK_LENGTH);

    return SRIX_NO_ERROR;
}

SrixError NfcWriteBlock(NfcReader reader[static 1], SrixBlock block[static 1], const uint8_t blockNum) {
    /* SRIX write command */
    const uint8_t writeCommand[] = {
            SRIX_WRITE_BLOCK,
            blockNum,
            block->block[0],
            block->block[1],
            block->block[2],
            block->block[3]
    };

    /* Array where save read block */
    SrixBlock check;

    /* Write while data aren't correct */
    do {
        /* Check tag presence */
        if (nfc_initiator_target_is_present(reader->libnfc_reader, (void *) 0) < 0) {
            SrixError error = nfcSrix4kInit(reader);
            if (SRIX_IS_ERROR(error)) {
                return error;
            }
        }

        /* Write data */
        nfcExchange(reader->libnfc_reader, writeCommand, 6, (void *) 0, 0);

        /* Check written data */
        NfcReadBlock(reader, &check, blockNum);
    } while (memcmp(block, &check, SRIX_BLOCK_LENGTH) != 0);

    return SRIX_NO_ERROR;
}

#undef SRIX_GET_UID
#undef SRIX_READ_BLOCK
#undef SRIX_WRITE_BLOCK