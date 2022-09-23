void kernel_main()
{
    volatile char* video_mem = (volatile char*) 0xb8000;
    video_mem = 'G';
    video_mem[1] = 2;
    kernel_halt();
}

void kernel_halt()
{
    for (;;);
}