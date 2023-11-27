# shivers
Shivers is a Discord bot made for hobby.

## Requirements
+ Linux
+ A C compiler (clang is recommended)
+ OpenSSL and its headers

## Building
```sh
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
	+ image: Image manipulation library
	+ json: JSON library
	+ network: Network library that contains websocket part and HTTP request
	+ utils: Utility library which used in source files or other internal libraries

+ include: Header files of everything

## Code
This project follows GNU99 standard and tries not to use external libraries, so includes some libraries that developed to use in this project.

## Disclaimer
This project is made using C, so it may be unstable or insecure. Contributors of this project cannot be held responsible for results of usage.

## License
This project is licensed under [BSD-3-Clause](./LICENSE) license.

