FROM ubuntu:latest

WORKDIR /OS

COPY . /OS

RUN sed -i 's/\r$//' ./SysDependensies/Dev.sh && chmod +x ./SysDependensies/Dev.sh && chmod +x ./SysDependensies/dotnet-install.sh && \
    ./SysDependensies/Dev.sh && mkdir build && cd build && cmake --build .