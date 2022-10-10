 ./configure --enable-smp \
              --enable-cpu-level=6 \
              --enable-all-optimizations \
              --enable-pci \
              --enable-gdb-stub \
              --enable-debugger \
              --enable-disasm \
              --enable-logging \
              --enable-fpu \
              --enable-3dnow \
              --enable-sb16=dummy \
              --enable-cdrom \
              --enable-x86-debugger \
              --enable-iodebug \
              --disable-docbook \
              --enable-magic-breakpoint
              --with-x --with-x11 --with-term
make
make install