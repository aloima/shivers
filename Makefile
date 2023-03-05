CC = clang
CFLAGS = --optimize=3 --warn-all --warn-extra --std=c99

.PHONY: compile clean

compile:
	$(CC) $(CFLAGS) $(wildcard src/**/*.c src/*.c libs/**/*.c) --include-directory=include --output=shivers

clean:
	rm shivers
