FROM debian:bullseye-slim AS build

WORKDIR /tmp

ENV PREFIX="/opt/cross"
ENV TARGET=i686-elf
ENV PATH="$PREFIX/bin:$PATH"

RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.xz
RUN wget https://ftp.gnu.org/gnu/gcc/gcc-10.4.0/gcc-10.4.0.tar.xz
RUN wget https://github.com/bochs-emu/Bochs/archive/refs/tags/REL_2_7_FINAL.zip
RUN tar -xf binutils-2.39.tar.xz
RUN tar -xf gcc-10.4.0.tar.xz

RUN apt update
RUN apt install -y build-essential bison flex libgmp3-dev \
                  libmpc-dev libmpfr-dev texinfo libisl-dev wget make \
                  libncurses-dev xorg-dev glew-utils unzip x11-utils x11-common

RUN mkdir build-binutils && \
    cd build-binutils && \
    ../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror &&\
    make && \
    make install

RUN mkdir build-gcc &&\
    cd build-gcc && \
    ../gcc-10.4.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers && \
    make all-gcc && \
    make all-target-libgcc && \
    make install-gcc && \
    make install-target-libgcc

RUN unzip REL_2_7_FINAL.zip
WORKDIR /tmp/Bochs-REL_2_7_FINAL/bochs

RUN ./configure \
              --prefix=/opt/bochs \
              --build=x86_64 \
              --host=x86_64 \
              --target=x86_64 \
              --enable-cpu-level=6 \
              --enable-pci \
              --enable-gdb-stub \
              --enable-logging \
              --enable-fpu \
              --enable-sb16=dummy \
              --enable-cdrom \
              --enable-x86-debugger \
              --enable-iodebug \
              --disable-docbook \
              --with-x11 

RUN make
RUN make install

FROM debian:bullseye-slim AS target

COPY --from=build /opt/cross /opt/cross
COPY --from=build /opt/bochs /opt/bochs
RUN apt update
RUN apt install -y qemu-system-i386 nasm git gdb dos2unix x11-utils x11-common