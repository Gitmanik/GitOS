//
// Created by Pawel Reich on 1/10/25.
//

extern "C" {
#include "stdio.h"
#include "string.h"
}

void process_command(char * str) {

    if (strlen(str) == 4) { // TODO: Implement strcmp or smth
        if (strncmp(str, "int3", 4) == 0) {
            asm volatile("int $3");
            return;
        }
        if (strncmp(str, "ping", 4) == 0) {
            puts("pong\n");
            return;
        }
    }

    printf("Unrecognized command: %s\n", str);
}

int main() {
    printf("GitOS Zofia - Build %s %s\n", __DATE__, __TIME__);

    char buffer[1024] {0};

    while (true) {
        putc('>');
        char c = 0;
        int idx = 0;
        while (true) {
            if (idx >= 1024)
                break;

            do {
                c = getc();
            } while (c == 0);

            if (c == '\r') {
                break;
            }
            buffer[idx] = c;
            putc(c);
            idx++;
        }
        buffer[idx] = 0;
        putc('\n');

        process_command(buffer);
    }
    return 0;
}
