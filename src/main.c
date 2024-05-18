#include "include/enums.h"
#include "include/embed.h"
#include "include/extract.h"
#include "include/status_codes.h"
#include "include/pseudo_aleatory_generator.h"

int main(int argc, char * argv[]){
    if (argc < 2){
        return EXIT_ILLEGAL_ARGUMENTS;
    }

    // Parse program mode: EMBED/EXTRACT
    program_mode mode = get_program_mode(argv[1]);
    if (mode == UNSPECIFIED_MODE)
        return EXIT_ILLEGAL_ARGUMENTS;

    argv += 2;
    argc -= 2;
    int status;
    if (mode == EMBED_MODE)
        status = embed(argc, argv);
    else if (mode == EXTRACT_MODE)
        status = extract(argc, argv);

    return status;
}