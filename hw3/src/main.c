#include <stdio.h>
#include "sfmm.h"

int main(int argc, char const *argv[]) {
    sf_mem_init();

    double* ptr = sf_malloc(3012);

    // *ptr = 320320320e-320;
    sf_show_heap();
    sf_realloc(ptr,2500);


    printf("%e\n", *ptr);

   // sf_free(ptr);

    sf_mem_fini();

    return EXIT_SUCCESS;
}
