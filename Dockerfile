FROM debian:bullseye-slim

RUN apt update
RUN apt install -y build-essential bison flex libgmp3-dev \
                  libmpc-dev libmpfr-dev texinfo libisl-dev wget make dox2unix

RUN apt install -y x11-utils x11-common x11-apps

RUN apt install -y qemu-system-i386
RUN apt install -y nasm

WORKDIR /tmp

ENV PREFIX="/opt/cross"
ENV TARGET=i686-elf
ENV PATH="$PREFIX/bin:$PATH"

RUN wget https://ftp.gnu.org/gnu/binutils/binutils-2.39.tar.xz
RUN wget https://ftp.gnu.org/gnu/gcc/gcc-10.4.0/gcc-10.4.0.tar.xz
 
RUN tar -xf binutils-2.39.tar.xz
RUN mkdir build-binutils && \
    cd build-binutils && \
    ../binutils-2.39/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror &&\
    make && \
    make install

RUN tar -xf gcc-10.4.0.tar.xz
RUN mkdir build-gcc &&\
    cd build-gcc && \
    ../gcc-10.4.0/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers && \
    make all-gcc && \
    make all-target-libgcc && \
    make install-gcc && \
    make install-target-libgcc

RUN rm -rf /tmp/*