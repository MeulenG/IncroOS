#!/bin/bash

# Function to check if a command exists
check_command_installed() {
  command -v "$1" > /dev/null 2>&1
}

# Function to install build dependencies
install_build_dependencies() {
  local packages=("git" "gcc" "g++" "nasm" "make" "curl" "cmake" "ninja-build" "python3" "xorg-dev" "gmp" "mpfr" "libmpc")
  echo "** installing build dependencies"
  for package in "${packages[@]}"; do
    if check_command_installed "$package"; then
      echo "** $package is already installed, skipping installation."
    else
      apt-get -y -qq install "$package"
    fi
  done
}

# Function to install Bochs
install_bochs() {
  if check_command_installed "bochs"; then
    echo "** Bochs is already installed, skipping installation."
  else
    echo "** downloading and installing Bochs"
    curl -O http://downloads.sourceforge.net/project/bochs/bochs/2.7/bochs-2.7.tar.gz
    tar -xzvf bochs-2.7.tar.gz
    cd bochs-2.7
    ./configure --with-sdl2 --with-x11 --enable-plugins --enable-debugger --enable-smp --enable-x86-64 --enable-svm --enable-avx --enable-long-phy-address --enable-all-optimizations --enable-ne2000  --enable-pnic --enable-e1000 --enable-usb --enable-usb-ohci --enable-usb-ehci --enable-usb-xhci --enable-raw-serial
    make
    make install
    cd ..
  fi
}

setup_cross_linux_x86_preparation() {
    export PREFIX="$HOME/opt/cross"
    export TARGET=i686-elf
    export PATH="$PREFIX/bin:$PATH"
}

install_binutils() {
  if check_command_installed "i686-elf-gcc"; then
    echo "** Cross-Compiler is already setup, skipping"
  else
    echo "** Setting up Binutils"
    cd $HOME/src
    mkdir build-binutils
    curl -O https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
    tar -xf binutils-2.40.tar.gz
    cd build-binutils
    ../binutils-2.40/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
    make
    make install
}

install_gcc() {
  if check_command_installed "i686-elf-gcc"; then
    echo "** Cross-Compiler is already setup, skipping"
  else
    echo "** Setting up Cross-Compiler"
    cd $HOME/src
    mkdir build-gcc
    curl -O https://ftp.gnu.org/gnu/gcc/gcc-12.3.0/gcc-12.3.0.tar.gz
    tar -xf gcc-12.3.0.tar.gz
    cd build-gcc
    ../gcc-12.3.0.tar.gz/configure --target=$TARGET --prefix="$PREFIX" --disable-nls --enable-languages=c,c++ --without-headers
    make all-gcc
    make all-target-libgcc
    make install-gcc
    make install-target-libgcc
}

# Main script
apt-get update -yqq
install_build_dependencies
setup_cross_linux_x86_preparation
install_bochs
install_binutils
install_gcc