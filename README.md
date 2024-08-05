# MFOS

This project is purely for educational purposes, and to satisfy my own interests. It has no real usage as of right now.

## Screenshots

![Screenshot](./docs/Bochs.png) <br />
<div id="top"></div>


## Run Locally

Clone the project
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
./configure --with-sdl2 --with-x11 --enable-plugins --enable-debugger --enable-smp --enable-x86-64 --enable-svm --enable-avx --enable-long-phy-address --enable-all-optimizations --enable-ne2000  --enable-pnic --enable-e1000 --enable-usb --enable-usb-ohci --enable-usb-ehci --enable-usb-xhci --enable-raw-serial
```

<!-- ## Features

- Light/dark mode toggle
- Live previews
- Fullscreen mode
- Cross platform -->

<!-- 
## Roadmap

- Additional browser support

- Add more integrations -->



## Contributing

Contributions are always welcome!


### Built With
* [osdev](https://wiki.osdev.org)
* [Brokenthorn](http://www.brokenthorn.com/Resources/)
* [CMake](https://cmake.org/)
* [NASM](https://nasm.us/)
* [BOCHS](https://bochs.sourceforge.io)

## Authors

- [@MeuleG](https://www.github.com/MeulenG)