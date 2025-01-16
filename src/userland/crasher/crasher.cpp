//
// Created by Pawel Reich on 1/16/25.
//

int main(int argc, char *argv[]) {
    (void)(argc);
    (void)(argv);
    __asm__ volatile("int $3");
    return 0;
}
