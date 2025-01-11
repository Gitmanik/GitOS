//
// Created by Pawel Reich on 1/10/25.
//

extern "C" {
#include "stdio.h"
}

int main() {
    puts("GitOS Zofia\n");
    puts("Build: ");
    puts(__DATE__);
    puts(" ");
    puts(__TIME__);
    puts("\n");

    while (true) {
        putc('>');
        char c = 0;
        while (c != '\r') {
            do {
                c = getc();
            } while (c == 0);
            putc(c);
        }
        putc('\n');
    }
    return 0;
}