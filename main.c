#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include "srix.h"


/**
 * Print help message.
 * @param executable name of executable
 */
static void printUsage(const char *executable) {
    printf("Usage: %s [-h] [-p] [-r file] [-w file] [-c] [-o]\n\n", executable);
    printf("Options:\n");
    printf("  -h        show this help message\n");
    printf("  -p        print information about NFC tag\n");
    printf("  -r file   read eeprom from a file, if not present read from NFC tag\n");
    printf("  -w file   write eeprom to a file\n");
    printf("  -c        write changes to NFC tag eeprom\n");
    printf("  -o        reset SRIX4K OTP blocks\n");
}


/**
 * Initialize srix from NFC.
 * @param srix struct to initialize
 * @return boolean result
 */
static bool readFromNfc(Srix *srix) {
    /* Get readers number */
    size_t readersNumber = NfcGetReadersCount(srix);

    /* Exit if no readers available */
    if (readersNumber == 0) {
        fprintf(stderr, "Unable to find an NFC reader\n");
        return false;
    }

    /* Print all readers */
    printf("Readers:\n");
    for (size_t i = 0; i < readersNumber; i++) {
        printf("[%" PRIu8 "] -> %s\n", (uint8_t) i, NfcGetDescription(srix, (int) i));
    }

    /* Reader selector */
    uint8_t targetReader = 0;
    if (readersNumber > 1) {
        printf("Found %zu readers available\n", readersNumber);

        /* Retry while input is out of range */
        do {
            printf("Insert the target reader [0-%zu]: ", readersNumber - 1);
            /* While input is invalid, clean input and retry */
            char buffer[8];
            fgets(buffer, sizeof(buffer), stdin);
            targetReader = strtol(buffer, (void *) 0, 10);
        } while (targetReader >= readersNumber);
    }

    /* Init nfc */
    if (!SrixNfcInit(srix, targetReader)) {
        /* If result is null, print error */
        fprintf(stderr, "Selected reader is invalid\n");
        return false;
    }

    return true;
}


/**
 * Initialize srix from a file.
 * @param srix struct to initialize
 * @param filename name of file
 * @return boolean result
 */
static bool readFromFile(Srix *srix, char *filename) {
    FILE *eeprom = fopen(filename, "r");
    if (!eeprom) {
        fprintf(stderr, "Unable to read input file\n");
        return false;
    }

    // Read blocks
    uint32_t blocks[SRIX4K_BLOCKS];
    for (int i = 0; i < SRIX4K_BLOCKS; i++) {
        uint8_t buffer[SRIX_BLOCK_LENGTH];
        if (fread(buffer, sizeof(uint8_t), SRIX_BLOCK_LENGTH, eeprom) != SRIX_BLOCK_LENGTH) {
            fprintf(stderr, "Incorrect read value from input file\n");
            fclose(eeprom);
            return false;
        }

        blocks[i] = buffer[0] << 24 | buffer[1] << 16 | buffer[2] << 8 | buffer[3];
    }

    // Read UID
    uint8_t uidBytes[SRIX_UID_LENGTH];
    if (fread(uidBytes, sizeof(uint8_t), SRIX_UID_LENGTH, eeprom) != SRIX_UID_LENGTH) {
        fprintf(stderr, "Incorrect read value from input file\n");
        fclose(eeprom);
        return false;
    }

    uint64_t uid = (uint64_t) uidBytes[7] << 56U | (uint64_t) uidBytes[6] << 48U | (uint64_t) uidBytes[5] << 40U |
                   (uint64_t) uidBytes[4] << 32U | (uint64_t) uidBytes[3] << 24U | (uint64_t) uidBytes[2] << 16U |
                   (uint64_t) uidBytes[1] << 8U | (uint64_t) uidBytes[0];

    /* Save data to SRIX struct */
    SrixMemoryInit(srix, blocks, uid);
    fclose(eeprom);
    return true;
}


/**
 * Save data to a file.
 * @param srix struct to save
 * @param filename name of file
 * @return boolean result
 */
static bool writeToFile(Srix *srix, char *filename) {
    FILE *outputFile = fopen(filename, "w");
    if (!outputFile) {
        fprintf(stderr, "Unable to open output file\n");
        return false;
    }

    // Write blocks
    for (int i = 0; i < SRIX4K_BLOCKS; i++) {
        uint32_t block = *SrixGetBlock(srix, i);
        uint8_t buffer[SRIX_BLOCK_LENGTH] = {block >> 24, block >> 16, block >> 8, block};

        if (fwrite(buffer, sizeof(uint8_t), SRIX_BLOCK_LENGTH, outputFile) != SRIX_BLOCK_LENGTH) {
            fprintf(stderr, "Incorrect written value to output file\n");
            fclose(outputFile);
            return false;
        }
    }

    // Write UID
    uint64_t uid = SrixGetUid(srix);
    uint8_t uidBytes[SRIX_UID_LENGTH] = {uid, uid >> 8, uid >> 16, uid >> 24, uid >> 32,
                                         uid >> 40, uid >> 48, uid >> 56};
    if (fwrite(uidBytes, sizeof(uint8_t), SRIX_UID_LENGTH, outputFile) != SRIX_UID_LENGTH) {
        fprintf(stderr, "Incorrect written value to output file\n");
        fclose(outputFile);
        return false;
    }

    fclose(outputFile);
    return true;
}

int main(int argc, char *argv[]) {
    /* Check if there are arguments */
    if (argc == 1) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    bool printInformation = false;
    char *readFile = (void *) 0;
    char *writeFile = (void *) 0;
    bool writeTag = false;
    bool resetOTP = false;

    /* Parse input arguments */
    int param;
    while ((param = getopt(argc, argv, "hpr:w:co")) != -1) {
        switch (param) {
            case 'h':
                printUsage(argv[0]);
                return EXIT_SUCCESS;
            case 'p':
                printInformation = true;
                break;
            case 'r':
                readFile = optarg;
                break;
            case 'w':
                writeFile = optarg;
                break;
            case 'c':
                writeTag = true;
                break;
            case 'o':
                resetOTP = true;
                break;
            default:
                printUsage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    Srix *srix = SrixNew();
    if (srix == (void *) 0) {
        fprintf(stderr, "Unable to allocate memory for SRIX\n");
        return EXIT_FAILURE;
    }

    /* Initialize NFC if read tag or write tag is enabled */
    if (!readFile || writeTag) {
        if (!readFromNfc(srix)) {
            return EXIT_FAILURE;
        }
    }

    /* Get data from file */
    if (readFile) {
        if (!readFromFile(srix, readFile)) {
            return EXIT_FAILURE;
        }
    }

    /* Print information in stdout */
    if (printInformation) {
        printf("UID: %lu\n\n", SrixGetUid(srix));

        printf("EEPROM:\n");
        for (int i = 0; i < SRIX4K_BLOCKS; i++) {
            printf("[%02X] -> %08X", i, *SrixGetBlock(srix, i));
        }
    }

    /* Reset OTP blocks */
    if (resetOTP) {
        /* If at least one OTP block is different than
         * 0xFFFFFFFF, reset OTP.
         * OTP Blocks: 0x00, 0x01, 0x02, 0x03, 0x04
         */
        bool toReset = false;
        for (int i = 0x00; i <= 0x04; i++) {
            if (*SrixGetBlock(srix, i) != 0xFFFFFFFF) {
                toReset = true;
            }
        }

        if (toReset) {
            /* Decrease block 6 */
            uint32_t block6old = *SrixGetBlock(srix, 0x06);
            uint32_t block6 =
                    (block6old << 24) | ((block6old & 0x0C) << 8) | ((block6old & 0x30) >> 8) | (block6old >> 24);

            uint32_t toDecrease = 0x00200000;
            if (block6 < toDecrease) {
                fprintf(stderr, "Unable to decrease block6 counter");
                return EXIT_FAILURE;
            } else {
                /* If result will be higher than 0, subtract. */
                block6 -= toDecrease;
            }

            block6 = (block6 << 24) | ((block6 & 0x0C) << 8) | ((block6 & 0x30) >> 8) | (block6 >> 24);

            SrixModifyBlock(srix, block6, 0x06);
            SrixModifyBlock(srix, 0xFFFFFFFF, 0x00);
            SrixModifyBlock(srix, 0xFFFFFFFF, 0x01);
            SrixModifyBlock(srix, 0xFFFFFFFF, 0x02);
            SrixModifyBlock(srix, 0xFFFFFFFF, 0x03);
            SrixModifyBlock(srix, 0xFFFFFFFF, 0x04);
        }
    }

    /* Write result to file */
    if (writeFile) {
        if (!writeToFile(srix, writeFile)) {
            return EXIT_FAILURE;
        }
    }

    /* Write result to tag */
    if (writeTag) {
        if (SrixWriteBlocks(srix) != SRIX_SUCCESS) {
            fprintf(stderr, "Unable to write blocks to SRIX4K");
        }
    }

    /* Delete srix at the end */
    SrixDelete(srix);

    return EXIT_SUCCESS;
}