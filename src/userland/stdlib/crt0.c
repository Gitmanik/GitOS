//
// Created by Pawel Reich on 1/16/25.
//
#include "misc.h"

extern void main(int argc, char** argv);

void crt0() {
    int argc = 0;
    char** argv = 0;
    get_process_arguments(&argc, &argv);
    main(argc, argv);
}