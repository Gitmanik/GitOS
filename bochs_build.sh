 #Requires build-essential libncurses-dev X11 server xorg-dev glew-utils
 #https://bochs.sourceforge.io/doc/docbook/user/compiling.html
 ./configure  --enable-cpu-level=6 \
              --enable-pci \
              --enable-gdb-stub \
              --enable-logging \
              --enable-fpu \
              --enable-sb16=dummy \
              --enable-cdrom \
              --enable-x86-debugger \
              --enable-iodebug \
              --disable-docbook \
              --with-x11 \
make
sudo make install