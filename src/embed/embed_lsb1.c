#include "embed_lsb1.h"
#include "get_file_size.h"
#include "print_error.h"
#include <stdio.h>

#define HEADER_SIZE 54
#define EXTENSION_SIZE 4 


status_code embed_lsb1(char* in_file_path, char* p_file_path, char* out_file_path) {
    
    off_t in_file_size = get_file_size(in_file_path); 
    off_t p_file_size = get_file_size(p_file_path);
    
    // queremos *8 porque es 1 bit por cada byte
    if(in_file_size*8 > (p_file_size-HEADER_SIZE-EXTENSION_SIZE)){
        // TODO: Show maximum secret size for bmp
        print_error("El archivo bmp no puede albergar el archivo a ocultar completo\n");
        return ILLEGAL_ARGUMENTS; //cambiar
    }

    // abrimos archivos
    FILE * in_file = fopen(in_file_path, "r");
    if(in_file == NULL){
        print_error("Error al abrir el archivo a ocultar\n");
        return ILLEGAL_ARGUMENTS; //cambiar
    }

    FILE * p_file = fopen(p_file_path, "r");
    if(p_file == NULL){
        print_error("Error al abrir el archivo a bmp\n");
        return ILLEGAL_ARGUMENTS; //cambiar
    }

    FILE * out_file = fopen(out_file_path, "w");
    if(out_file == NULL){
        print_error("Error al abrir el archivo de salida\n");
        return ILLEGAL_ARGUMENTS; //cambiar
    }

    // Copiar header
    unsigned char header[HEADER_SIZE];
    fread(header, sizeof(unsigned char), HEADER_SIZE, in_file);
    fwrite(header, sizeof(unsigned char), HEADER_SIZE, out_file);

    // copiar payload del bmp modificando el lsb con el in al archivo a ocultar
    // cada bit de in va al lsb de un byte del bmp
    unsigned char in_byte;
    unsigned char p_byte;
    unsigned char out_byte;
    while(fread(&in_byte, sizeof(unsigned char), 1, in_file) == 1){
        for(int i = 8 ; i > 0 ; i--) {
            fread(&p_byte, sizeof(unsigned char), 1, p_file);
            out_byte = (p_byte & 0xFE) | ((in_byte >> (i-1)) & 0x01);
            fwrite(&out_byte, sizeof(unsigned char), 1, out_file);
        }
    }

    //cerramos archivos
    fclose(in_file);
    fclose(p_file);
    fclose(out_file);

    return SUCCESS;
}

