CC = clang
CFLAGS = --optimize=3 --warn-all --std=gnu99
LIBRARIES = -lm -lssl -lcrypto -lpthread `pkg-config vips --cflags --libs`

.PHONY: compile clean

compile:
	$(CC) $(CFLAGS) $(LIBRARIES) src/**/*.c src/*.c libs/**/*.c --include-directory=include --output=shivers

clean:
	rm shivers
