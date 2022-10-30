#!/bin/bash

# Dev-libraries
echo "** installing build dependencies"
apt-get update -yqq
apt-get -y -qq install git gcc g++ nasm make curl cmake ninja-build python3
