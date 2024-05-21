#ifndef _PARAMS_PARSER_H
#define _PARAMS_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "embed.h"
#include "enums.h"

typedef struct params {
    // Required
    program_mode mode;
    char * in_file;
    char * p_bitmap_file;
    char * out_bitmap_file;
    steg_alg steg;
    // Optional
    encryption_alg encryption; // default = AES128
    block_chaining_mode chaining; // default = CBC
    char * password;
} tparams;

typedef tparams * params_ptr; 

params_ptr init_params();
params_ptr set_params(int argc, char * argv[]);



#endif