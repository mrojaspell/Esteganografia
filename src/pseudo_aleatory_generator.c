#include <time.h>
#include <stdlib.h>

#include "include/pseudo_aleatory_generator.h"

void setRandomSeed(){
    srand(time(0));
}