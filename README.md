# OMOS


<div id="top"></div>

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#built-with">Built With</a></li>
      </ul>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#prerequisites">Prerequisites</a></li>
        <li><a href="#installation">Installation</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#contributing">Contributing</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgments">Acknowledgments</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project


My first OS written in Assembly and C in order to learn OS Development. I dedicate all my spare-time to developing this.


<p align="right">(<a href="#top">back to top</a>)</p>



### Built With


* [osdev](https://wiki.osdev.org)
* [C](https://www.learn-c.org/)
* [MakeFile](https://makefile.site/)
* [NASM](https://nasm.us/)
* [QEMU](https://www.qemu.org/)
<p align="right">(<a href="#top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

In order to setup this project, you need a cross-compiler, gcc compiler and NASM(assembly compiler). 

### Prerequisites


* gcc (C/C++ compiler)
  ```sh
  sudo apt install build-essential
  ```

* NASM (x86 compiler)
  ```sh
  sudo apt install nasm
  ```
* Make
  ```sh
  sudo apt install make
  ```
* QEMU (emulator)
  ```sh
  sudo apt install qemu
  ```


You can use an already written cross-compiler or write your own, if you dont want to write your own, follow this link to setup a cross-compiler[Cross-Compiler](https://wiki.osdev.org/GCC_Cross-Compiler)

### Installation

1. Clone the repo
   ```sh
   git clone https://github.com/MeulenG/OMOS.git
   ```
The next thing you wanna do is modify the Makefile, because you are going to want to change the cross-compiler path to your own.
After having succesfully cloned the repo, you need to go into the root directory and type "make os-image.bin && make kernel.bin && make kernel.elf" and thereafter you can run it by simply typing "make run".

<p align="right">(<a href="#top">back to top</a>)</p>


## Usage

This project is purely for my own enjoyment and to learn, there is no specific usage for it right now.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ROADMAP -->
## Roadmap

- [ ] Disk Driver
- [ ] File System
- [ ] User mode
- [ ] Text Editor
- [ ] Multithreaded Kernel
- [ ] Scheduling and Multiple Processes
- [ ] Networking

See the [open issues](https://github.com/MeulenG/OMOS/issues) for a full list of proposed features (and known issues).

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTRIBUTING -->
## Contributing

Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE.txt` for more information.

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- CONTACT -->
## Contact

Oliver Meulengracht - [Linkedin](https://www.linkedin.com/in/olivermeulengracht/) - Mollern2000@outlook.dk

Project Link: [OMOS](https://github.com/MeulenG/OMOS.git)

<p align="right">(<a href="#top">back to top</a>)</p>



<!-- ACKNOWLEDGMENTS -->
## Acknowledgments

Use this space to list resources you find helpful and would like to give credit to. I've included a few of my favorites to kick things off!

* [Brokenthorn](http://brokenthorn.com/)
<p align="right">(<a href="#top">back to top</a>)</p>