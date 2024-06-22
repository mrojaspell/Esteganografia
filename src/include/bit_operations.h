#ifndef BIT_OPERATIONS_H
#define BIT_OPERATIONS_H

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "status_codes.h"

// Get the last bit of the following byte
// This function advances the file pointer
status_code get_last_bit(FILE * file, bool * bit);

// Set the last bit of the following byte
// This function advances the file pointer
status_code set_last_bit(FILE * file, bool bit);

// Invert the last bit of the following byte
// This function advances the file pointer
status_code invert_last_bit(FILE * file);

// Retrieve the LSBI pattern of the last read byte
// This function DOES NOT advance the file pointer
status_code get_LSBI_pattern(FILE * file, uint8_t * pattern);

// Invert bit if pattern is marked to invert
// This function DOES NOT advance the file pointer
status_code invert_last_bit_by_pattern(FILE * file, bool patterns[4]);

#endif