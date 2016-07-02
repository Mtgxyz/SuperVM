
CC = gcc
CFLAGS=-O3

all: svmln expdump emulator svmas

.PHONY: as clean run

svmln: svmln.c
	$(CC) -g -o bin/$@ $^ $(CFLAGS)

expdump: expdump.c mnemonics.c disassembler.c
	$(CC) -g -o bin/$@ $^ $(CFLAGS)

emulator: emulator.c vm.c
	$(CC) -g -o bin/$@ $^ -lSDL $(CFLAGS)

svmas:
	$(MAKE) -C as

test: exp
	./exp -o test.exp $(ARGS)
	hexdump -C test.exp 

run: test.exp
	./bin/emulator -dV $^
	
%.exp: %.bin
	./bin/explink -o $@ -c $^

%.bin: %.asm
	./bin/as -o $@ -L $^

clean:
	rm *.bin
