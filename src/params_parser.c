#include "include/exit.h"


#define REQUIRED_PARAMS_NO 4
#define REQUIRED_ARGS_NO (REQUIRED_PARAMS_NO * 2)

static int mode = 0;

static struct option long_opts[] = 
{
    {"embed", no_argument, &mode, EMBED_MODE},
    {"extract", no_argument, &mode, EXTRACT_MODE},
    {"in", required_argument, NULL, 'i'},
    {"p", required_argument, NULL, 'p'},
    {"out", required_argument, NULL, 'o'},
    {"steg", required_argument, NULL, 's'},
    {"a", required_argument, NULL, 'a'},
    {"m", required_argument, NULL, 'm'},
    {"pass", required_argument, NULL, 'c'},
    {0, 0, 0, 0}
};

params_ptr init_params() {
    params_ptr params = malloc(sizeof(tparams));

    if(params == NULL) {
        printf("Error when trying to allocate memory for parameters");
        exit_handler(INSUFFICIENT_MEMORY, NULL);
    }
    params->mode = UNSPECIFIED_MODE;
    params->in_file = NULL;
    params->p_bitmap_file = NULL;
    params->out_bitmap_file = NULL;
    params->steg = UNSPECIFIED_STEG;
    params->encryption = UNSPECIFIED_ENC;
    params->chaining = UNSPECIFIED_CHAIN;
    params->password = NULL;
    return params;
}

params_ptr set_params(int argc, char * argv[]){
    params_ptr params = init_params();

    int opt;
    while((opt=getopt_long_only(argc, argv, "i:p:o:s:a:m:c:",long_opts,NULL)) != -1) {
        switch(opt) {
            case 0:
                if(mode == 0) {
                    exit_handler(ILLEGAL_ARGUMENTS, params);
                }
                params->mode = mode;
                break;
            case 'i':
                params->in_file = optarg;
                break;
            case 'p':
                params->p_bitmap_file = optarg;
                break;
            case 'o':
                params->out_bitmap_file = optarg;
                break;
            case 's':
                params->steg = get_steg_alg(optarg);
                break;
            case 'a':
                params->encryption = get_encryption_alg(optarg);
                break;
            case 'm':
                params->chaining = get_chaining_mode(optarg);
                break;
            case 'c':
                params->password = optarg;
                break;
            default:
                exit_handler(ILLEGAL_ARGUMENTS, params);
        }
    }

    // Check that the struct is valid
    if (params->mode == UNSPECIFIED_MODE) {
        exit_handler(MISSING_MODE, params);
    }

    if ((params->in_file == NULL || 
    params->p_bitmap_file == NULL || 
    params->out_bitmap_file == NULL || 
    params->steg == UNSPECIFIED_STEG) && params->mode == EMBED_MODE
    ) {
        exit_handler(MISSING_EMBED_ARGUMENTS, params);
    }
    if((params->p_bitmap_file == NULL || 
    params->out_bitmap_file == NULL || 
    params->steg == UNSPECIFIED_STEG) && params->mode == EXTRACT_MODE) {
        exit_handler(MISSING_EXTRACT_ARGUMENTS, params);
    }

    // tiene password
    if(params->password != NULL) {
        if(params->encryption == UNSPECIFIED_ENC) {
            params->encryption = AES128;
        }
        if(params->chaining == UNSPECIFIED_CHAIN) {
            params->chaining = CBC;
        }
    }
    // si no tiene password salteamos la encriptacion aunque de los argumentos de encripcion y encadenamiento

    return params;
}

