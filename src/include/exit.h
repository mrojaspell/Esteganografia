#ifndef _EXIT_H
#define _EXIT_H

#include "params_parser.h"
#include "color_print.h"

typedef enum status_code {
    SUCCESS,
    ILLEGAL_ARGUMENTS,
    INVALID_IN_FILE,
    INVALID_P_FILE,
    INVALID_OUT_PATH,
    INVALID_STEG_ALG,
    INSUFFICIENT_MEMORY,
    MISSING_EMBED_ARGUMENTS,
    MISSING_EXTRACT_ARGUMENTS,
    MISSING_MODE,
    // add more codes here
} status_code;

void exit_handler(status_code status, params_ptr params);

void success_exit();

void illegal_argument_exit();

void missing_embed_arguments_exit();

void missing_extract_arguments_exit();

void invalid_in_file_exit();

void invalid_p_file_exit() ;

void invalid_out_path_exit() ;

void invalid_steg_alg_exit();

void insufficient_memory_exit();

void print_embed_arguments();

void print_extract_arguments();

void print_use();

void missing_mode();

#endif