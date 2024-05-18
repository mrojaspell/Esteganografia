#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#include "include/embed.h"
#include "include/enums.h"

#define REQUIRED_PARAMS_NO 4
#define REQUIRED_ARGS_NO REQUIRED_PARAMS_NO * 2

struct params {
    // Required
    char * in_file;
    char * p_bitmap_file;
    char * out_bitmap_file;
    steg_alg steg;
    // Optional
    encryption_alg encryption; // default = AES128
    block_chaining_mode chaining; // default = CBC
    char * password;
};

status_code set_params(struct params * params, int argc, char * argv[]);
status_code set_param(struct params * params, char * arg_name, char * arg_value);

status_code embed(int argc, char * argv[]){
    

    // Initialize params
    struct params params = {0};

    // Set params
    status_code status = set_params(&params, argc, argv);
    if (status != EXIT_SUCCESS)
        return status;


    return EXIT_SUCCESS;
}

status_code set_params(struct params * params, int argc, char * argv[]){
    // Check correct number
    if (argc < REQUIRED_ARGS_NO * 2 || argc % 2 == 1)
        return EXIT_ILLEGAL_ARGUMENTS;

    // Set params struct
    for (int i = 0; i < argc; i+=2){
        int status = set_param(params, argv[i], argv[i+1]);
        if (status != EXIT_SUCCESS)
            return status;
    }

    // Check that the struct is valid
    if (params->in_file == NULL || params->p_bitmap_file == NULL || params->out_bitmap_file == NULL || params->steg == UNSPECIFIED_STEG)
        return EXIT_ILLEGAL_ARGUMENTS;

    return EXIT_SUCCESS;
}

status_code set_param(struct params * params, char * arg_name, char * arg_value){
    bool error_ocurred = false;
    if (!strcmp(arg_name, "-in")){
        params->in_file = arg_value;
    }
    else if (!strcmp(arg_name, "-p")){
        params->p_bitmap_file = arg_value;
    }
    else if (!strcmp(arg_name, "-out")){
        params->out_bitmap_file = arg_value;
    }
    else if (!strcmp(arg_name, "-steg")){
        params->steg = get_steg_alg(arg_value);
        error_ocurred = (params->steg == UNSPECIFIED_STEG);
    }
    else if (!strcmp(arg_name, "-a")){
        params->encryption = get_encryption_alg(arg_value);
        error_ocurred = (params->encryption == UNSPECIFIED_ENC);
    }
    else if (!strcmp(arg_name, "-m")){
        params->chaining = get_chaining_mode(arg_value);
        error_ocurred = (params->chaining == UNSPECIFIED_CHAIN);
    }
    else if (!strcmp(arg_name, "-pass")){
        params->password = arg_value;
    }
    else {
        error_ocurred = true;
    }

    if (error_ocurred)
        return EXIT_ILLEGAL_ARGUMENTS;
    return EXIT_SUCCESS;
}

