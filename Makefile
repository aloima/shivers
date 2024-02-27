ifeq ($(OS),Windows_NT)
	CC := gcc
	LIBRARIES := -lm -lssl -lcrypto -lpthread -lz -lfreetype -lws2_32 -lwsock32 -I/ucrt64/include/freetype2/
else
	CC := clang
	LIBRARIES := -lm -lssl -lcrypto -lpthread -lz -lfreetype -I/usr/include/freetype2/
endif

CFLAGS := --warn-all --std=gnu99

.PHONY: compile clean

compile:
	$(CC) $(CFLAGS) src/**/*.c src/*.c libs/**/*.c --include-directory=include --output=shivers $(LIBRARIES)

clean:
	rm shivers
