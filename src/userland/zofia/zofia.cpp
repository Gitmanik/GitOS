//
// Created by Pawel Reich on 1/10/25.
//

#include "graphics/framebuffer.hpp"

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

    if (strcmp("int3", str) == 0) {
        asm volatile("int $3");
        return;
    }
    if (strcmp("ping", str) == 0) {
        puts("pong\n");
        return;
    }

    if (strcmp("red", str) == 0) {
        for (int x = 0; x < 1024; x++) {
            for (int y = 0; y < 768; y++) {
                FramebufferGraphics::the()->draw_pixel(x, y, 0xff0000);
            }
        }
    }

    if (strcmp("clear", str) == 0) {
        FramebufferGraphics::the()->clear_screen();
    }


    char buf[1024] {0};
    int len = strlen(get_cwd());
    strcpy(buf, get_cwd());
    strcpy(buf + len, str);

    execprocess(buf);
}

int main(int argc, char** argv) {
    printf("GitOS Zofia - Build %s %s\n", __DATE__, __TIME__);

    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }

    char buffer[1024] {0};

    while (true) {
        memset(buffer, 0, 1024);
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
