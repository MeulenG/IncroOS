#!/bin/bash

# Function to check if a command exists
check_command_installed() {
  command -v "$1" > /dev/null 2>&1
}

# Function to install build dependencies
install_build_dependencies() {
  local packages=("git" "gcc" "g++" "nasm" "make" "curl" "cmake" "ninja-build" "python3" "xorg-dev")
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

# Main script
apt-get update -yqq
install_build_dependencies
install_bochs