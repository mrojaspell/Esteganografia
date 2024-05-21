#include <stdio.h>

#include "include/enums.h"
#include "include/embed.h"
#include "include/extract.h"
#include "include/pseudo_aleatory_generator.h"
#include "include/color_print.h"
#include "include/params_parser.h"

int main(int argc, char * argv[]){
    params_ptr params = set_params(argc, argv);

    printf("Mode: %d\n", params->mode);
    printf("In file: %s\n", params->in_file);
    printf("P bitmap file: %s\n", params->p_bitmap_file);
    printf("Out bitmap file: %s\n", params->out_bitmap_file);
    printf("Steg: %d\n", params->steg);
    printf("Encryption: %d\n", params->encryption);
    printf("Chaining: %d\n", params->chaining);
    if(params->password != NULL)
        printf("Password: %s", params->password);

    exit_handler(SUCCESS, params);
}