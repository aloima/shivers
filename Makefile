ifeq ($(OS),Windows_NT)
	CC := gcc
	LIBRARIES := -lm -lssl -lcrypto -lpthread -lz -lws2_32 -lwsock32
else
	CC := clang
	LIBRARIES := -lm -lssl -lcrypto -lpthread -lz
endif

CFLAGS := --optimize=3 --warn-all --std=gnu99

.PHONY: compile clean

compile:
	$(CC) $(CFLAGS) src/**/*.c src/*.c libs/**/*.c --include-directory=include --output=shivers $(LIBRARIES)

clean:
	rm shivers
