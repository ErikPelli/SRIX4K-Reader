#ifndef ERROR_H
#define ERROR_H

#include <stdint.h>

/**
 * Error codes enum.
 * SUCCESS = 0.
 * ERROR < 0.
 */
typedef enum {
    SRIX_SUCCESS,
    NFC_ERROR = INT8_MIN,
    SRIX_ERROR
} SrixErrorCode;

/**
 * Error structure that contains a description message.
 */
typedef struct SrixError {
    SrixErrorCode errorType;
    char const *message;
} SrixError;

#define SRIX_NO_ERROR                     ((SrixError) {.errorType = SRIX_SUCCESS})
#define SRIX_ERROR(type, errorMessage)    ((SrixError) {.errorType = (type), .message = (errorMessage)})
#define SRIX_IS_ERROR(isError)            ((isError).errorType != SRIX_SUCCESS)

/**
 * SRIX4K constants
 */
#define SRIX_BLOCK_LENGTH  4
#define SRIX_UID_LENGTH    8
#define SRIX4K_BLOCKS      128
#define SRIX4K_BYTES       512

#endif /* ERROR_H */