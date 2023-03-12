CC = clang
CFLAGS = --optimize=3 --warn-everything --std=c89 -lm

.PHONY: compile clean

compile:
	$(CC) $(CFLAGS) $(wildcard src/**/*.c src/*.c libs/**/*.c) --include-directory=include --output=shivers

clean:
	rm shivers
