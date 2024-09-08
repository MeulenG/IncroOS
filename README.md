# MFOS

This project is purely for educational purposes, and to satisfy my own interests. It has no real usage.

## Screenshots

![Screenshot](./docs/Bochs.png) <br />
<div id="top"></div>
(This is an old screenshot, will be updated ASAP)

## Run Locally

Clone the project locally along with the submodules(SSH is recommended):
```sh
git clone --recurse-submodules -j8 git@github.com:MeulenG/MFOS.git
```
```dev.sh``` script will make sure you have all the needed packages required to build the OS:
```sh
./dev.sh
```
Before any attempt is made to build, please do run the following script which updates and clones submodules:
```sh
./setup.sh
```
Additionally, you can install .NET by running:
```sh
./dotnet-install.sh
```
The recommended emulation software is Bochs, and it is also recommended that it is built from source and can be downloaded * [here](https://bochs.sourceforge.io). The recommended configuration options for the OS currently are:
```sh
./configure --with-x11 --enable-plugins --enable-debugger --enable-smp --enable-x86-64 --enable-svm --enable-avx --enable-long-phy-address --enable-all-optimizations --enable-ne2000  --enable-pnic --enable-e1000 --enable-usb --enable-usb-ohci --enable-usb-ehci --enable-usb-xhci --enable-raw-serial
```

Features

- Multistage Fat32 Bootloader
 
## Roadmap

- Kernel World



## Contributing

Contributions are always welcome!


### Built With
* [osdev](https://wiki.osdev.org)
* [Brokenthorn](http://www.brokenthorn.com/Resources/)
* [CMake](https://cmake.org/)
* [NASM](https://nasm.us/)
* [BOCHS](https://bochs.sourceforge.io)

## Authors

- [@MeulenG](https://www.github.com/MeulenG)