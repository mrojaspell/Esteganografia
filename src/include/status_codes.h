#ifndef STATUS_CODES_H
#define STATUS_CODES_H

typedef enum status_code {
    SUCCESS,
    ILLEGAL_ARGUMENTS,
    INVALID_IN_FILE,
    INVALID_P_FILE,
    INVALID_OUT_PATH,
    INVALID_STEG_ALG,
    FILE_OPEN_ERROR,
    FILE_READ_ERROR,
    FILE_WRITE_ERROR,
    SECRET_TOO_BIG,
    MEMORY_ERROR
    // add more codes here
} status_code;

#endif