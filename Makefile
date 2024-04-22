ifneq (, $(shell which gcc))
	CC := gcc
else ifneq (, $(shell which clang))
	CC := clang
endif

ifeq ($(OS),Windows_NT)
	LIBRARIES := -lm -lssl -lcrypto -lpthread -lz -lfreetype -lws2_32 -lwsock32 -lpsapi -I/ucrt64/include/freetype2/
else
	LIBRARIES := -lm -lssl -lcrypto -lpthread -lz -lfreetype -I/usr/include/freetype2/
endif

CFLAGS := --warn-all --std=gnu99

%.o: %.c check
	@mkdir -p build/$(<D)
	@$(CC) $(CFLAGS) -c $< --include-directory=include $(LIBRARIES) --output build/$@
	@echo Compiled $<.

compile: $(patsubst %.c,%.o,$(wildcard **/**/*.c **/*.c))
	@$(CC) $(CFLAGS) $(addprefix build/,$?) --include-directory=include --output=shivers $(LIBRARIES)
	@echo Compiled shivers.

check:
ifeq (,$(CC))
	@echo There is no compiler.
else
	@echo Compiler: $(CC)
endif

clean:
	rm build
	rm shivers
