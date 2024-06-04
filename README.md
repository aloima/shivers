# shivers
Shivers is a Discord bot made for hobby and educational purposes.

## Requirements

### Linux
+ A C compiler (clang is recommended)
+ OpenSSL and its headers
+ zlib and its headers
+ freetype and its headers

### Windows (WIP)
+ MSYS2
+ MSYS2 compiler (gcc is recommended)
+ OpenSSL and its headers for MSYS2
+ zlib and its headers for MSYS2
+ freetype and its headers for MSYS2

> Tested for MSYS2 UCRT64 environment, so Makefile is set for this environment, if your envionment is different, change it.

## Building
```bash
make
```

## Directories
+ src: Source files
	+ commands: Command files
	+ events: Event files
	+ utils: Utility files which used in source files

+ libs: Internal libraries
	+ database: Database library
	+ discord: Discord library which create connections with Discord
	+ hash: HashMap library
	+ png: PNG manipulation library
	+ json: JSON library
	+ network: Network library that contains websocket part and HTTP request
	+ utils: Utility library which used in source files or other internal libraries

+ include: Header files of everything

## Code
This project follows GNU99 standard and tries not to use external libraries, so includes some libraries that developed to use in this project.

## Disclaimer
This project is made using C, so it may be unstable. Contributors of this project cannot be held responsible for results of usage.

## License
This project is licensed under [BSD-3-Clause](./LICENSE) license.

