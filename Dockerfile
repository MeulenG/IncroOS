FROM ubuntu:latest AS build

WORKDIR /os-deps

COPY SysDependensies .

RUN sed -i 's/\r$//' ./Dev.sh && chmod +x ./Dev.sh && chmod +x ./dotnet-install.sh && \
    ./Dev.sh

#build cross-compiler
FROM build AS build-compiler

WORKDIR /os-compiler

COPY ./LLVM-Clang/llvm-project .

RUN cmake -S llvm -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release && cmake --build build

#build bochs
FROM build as build-emulator

WORKDIR /os-emulator

COPY ./SysEmulation .

RUN curl https://sourceforge.net/projects/bochs/files/bochs/2.6.9/bochs-2.6.9.tar.gz/download && gunzip -c bochs-2.6.9.tar.gz | tar -xvf - && cd bochs-2.6.9 \
    ./configure && make -j && sudo make install && sudo make unpack dlx && sudo make install dlx && \
    ./configure --with-x11 --with-sdl2 --enable-plugins --enable-debugger --enable-readline --enable-xpm --enable-show-ips --enable-assert-checks --enable-idle-hack --enable-smp --enable-3dnow --enable-x86-64 --enable-vmx --enable-svm --enable-avx --enable-x86-debugger --enable-alignment-check --enable-long-psy-address --enable-a20-pin --enable-large-ramfile --enable-all-optimizations --enable-cdrom --enable-gameport --enable-pnic --enable-e1000 --enable-pci --enable-usb --enable-usb-ochi --enable-raw-serial

# multistage
FROM build AS build-os

WORKDIR /os

COPY . .

RUN mkdir build && cd build && cmake .. && make