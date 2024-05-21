#include "include/exit.h"

typedef void (*error_print)(void);

static const error_print exit_prints[] = {
    success_exit,
    illegal_argument_exit,
    invalid_in_file_exit,
    invalid_p_file_exit,
    invalid_out_path_exit,
    invalid_steg_alg_exit,
    insufficient_memory_exit,
    missing_embed_arguments_exit,
    missing_extract_arguments_exit,
    missing_mode
};

void exit_handler(status_code status, params_ptr params) {
    exit_prints[status]();
    free(params);
    exit(status);
}

void success_exit() {
    color_print(stdout, GREEN, "Process exited and finished correctly\n");
}

void illegal_argument_exit() {
    color_print(stderr, RED, "Illegal arguments\n");
    print_use();
}

void missing_embed_arguments_exit() {
    color_print(stderr, RED, "Missing embed arguments\n");
    print_embed_arguments();
}

void missing_extract_arguments_exit() {
    color_print(stderr, RED, "Missing extract arguments\n");
    print_extract_arguments();
}

void invalid_in_file_exit() {
    color_print(stderr, RED, "Error in in file\n");
}

void invalid_p_file_exit() {
    color_print(stderr, RED, "Error in p file\n");
}

void invalid_out_path_exit() {
    color_print(stderr, RED, "Error in out path\n");
}

void invalid_steg_alg_exit() {
    color_print(stderr, RED, "Error in steganography algorithm\n");
}

void insufficient_memory_exit() {
    color_print(stderr, RED, "Insufficient memory\n");
}

void print_use() {
    print_embed_arguments();
    print_extract_arguments();
}

void print_embed_arguments() {
    color_print(stderr, YELLOW, "./stegobmp -embed -in <file> -p <bitmapfile> -out <output_bitmapfile> -steg <LSB1|LSB4|LSBI> "
    "-a <aes128|aes192|aes256|des> -m <ecb|cfb|ofb|cbc> -pass <password>\n");
}
void print_extract_arguments() {
    color_print(stderr, YELLOW, "./stegobmp -extract -p <bitmapfile> -out <output_bitmapfile> -steg <LSB1|LSB4|LSBI>"
    " -a <aes128|aes192|aes256|des> -m <ecb|cfb|ofb|cbc> -pass <password>\n");
}

void missing_mode() {
    color_print(stderr, RED, "Missing mode, use -extrct or -embed\n");
}

