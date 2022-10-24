FROM ubuntu:latest AS build

WORKDIR /os-deps

COPY . .


RUN sed -i 's/\r$//' ./SysDependensies/Dev.sh && chmod +x ./SysDependensies/Dev.sh && chmod +x ./SysDependensies/dotnet-install.sh && \
    ./SysDependensies/Dev.sh

FROM build

WORKDIR /os

COPY . .

RUN mkdir build && cd build && cmake .. && make