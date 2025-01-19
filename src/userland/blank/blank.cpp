//
// Created by Pawel Reich on 1/17/25.
//

extern "C" {
#include "misc.h"
#include "string.h"
}

int main(int argc, char** argv) {
    (void)(argc);
    (void)(argv);
    void* test = malloc(3);
    memcpy(test, &"OK\0", 3);
    if (memcmp((void *) &"OK\0", test, 3) == 0)
        printf("Allocation test at 0x%p: %s\n", test, test);
    else
        printf("Allocation test: UNSUCCESFUL\n");

    free(test);
    return 0;
}