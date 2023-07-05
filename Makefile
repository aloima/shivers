CC = clang
CFLAGS = --optimize=3 --warn-all --std=gnu99 -lm -lssl -lcrypto -lpthread

.PHONY: compile clean

compile:
	$(CC) $(CFLAGS) $(wildcard src/**/*.c src/*.c libs/**/*.c) --include-directory=include --output=shivers

clean:
	rm shivers
