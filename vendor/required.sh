#!/bin/bash

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

# main
apt-get update -yqq
install_build_dependencies