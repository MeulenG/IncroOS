FROM ubuntu:latest AS build

WORKDIR /os-deps

COPY SysDependensies .

RUN sed -i 's/\r$//' ./Dev.sh && chmod +x ./Dev.sh && chmod +x ./dotnet-install.sh && \
    ./Dev.sh

FROM build AS build-compiler

WORKDIR /os-compiler

COPY ./LLVM-Clang/llvm-project .

RUN cmake -S llvm -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release && cmake --build build

# multistage
FROM build AS build-os

WORKDIR /os

COPY . .

RUN mkdir build && cd build && cmake .. && make