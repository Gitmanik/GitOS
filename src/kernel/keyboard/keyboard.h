#pragma once

typedef int (*KEYBOARD_INIT_FUNCTION)();
struct keyboard
{
    KEYBOARD_INIT_FUNCTION init;
    char name[20];
    struct keyboard* next;
};

char keyboard_pop();
void keyboard_push(char c);
int keyboard_insert(struct keyboard* keyboard);