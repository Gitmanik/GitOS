//
// Created by Pawel Reich on 1/10/25.
//

extern "C" {
#include "stdio.h"
#include "string.h"
#include "misc.h"
}

const char* get_cwd() {
    static char cwd[] = "0:/";
    return cwd;
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

    char buf[1024] {0};
    int len = strlen(get_cwd());
    strcpy(buf, get_cwd());
    strcpy(buf + len, str);

    int res = execprocess(buf);
    if (res == 0) {
        printf("Executing process: %s", str);
        return;
    }

    printf("Unrecognized command: %s\n", str);
}

int main() {
    printf("GitOS Zofia - Build %s %s\n", __DATE__, __TIME__);

    char buffer[1024] {0};

    while (true) {
        printf("%s $", get_cwd());
        char c = 0;
        int idx = 0;
        while (true) {
            if (idx >= 1024)
                break;

            do {
                c = getc();
            } while (c == 0);

            if (c == 8) {
                if (idx == 0)
                    continue;
                idx--;
                buffer[idx] = 0;
                continue;
            }

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
