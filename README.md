# shivers
Shivers is a Discord bot made for hobby and educational purposes

## Requirements

### Linux
+ A C compiler (clang is recommended)
+ `OpenSSL` and its development headers
+ `zlib` and its development headers
+ `freetype` and its development headers

### Windows (WIP)
+ MSYS2
+ MSYS2 compiler (gcc is recommended)
+ `OpenSSL` and its development headers for MSYS2
+ `zlib` and its development headers for MSYS2
+ `freetype` and its development headers for MSYS2

> Tested for MSYS2 UCRT64 environment, so Makefile is set for this environment, if your envionment is different, change it.

## Building
* Clone the repository
* Create build directory using `mkdir build` and enter `cd build`
* Generate compilation files using `cmake .. --preset=release` or `cmake .. --preset=debug`
* Enter `cd release` or `cd debug`, depends on which preset you prepared
* Compile inside the directory using `make shivers`, then start the server using `./shivers`

## Directories
* `src`: Source files
  * `commands`: Command files
  * `events`: Event files
  * `utils`: Utility files which used in source files

* `libs`: Internal libraries
  * `database`: Database library
  * `discord`: Discord library which create connections with Discord
  * `hash`: Hash library, includes HashMap structure
  * `png`: PNG manipulation library
  * `json`: JSON library
  * `network`: Network library that contains websocket part and HTTP request
  * `utils`: Utility library which used in source files or other internal libraries

* `include`: Header files of everything

## Code
This project follows C23 standard and tries not to use external libraries, so includes some libraries that developed to use in this project.

## License
This project is licensed under [BSD-3-Clause](./LICENSE) license.
