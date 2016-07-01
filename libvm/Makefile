
CC = gcc

all: explink expdump emulator as

.PHONY: as clean run

explink: explink.c
	$(CC) -g -o bin/$@ $^

expdump: expdump.c mnemonics.c disassembler.c
	$(CC) -g -o bin/$@ $^

emulator: emulator.c vm.c
	$(CC) -g -o bin/$@ $^ -lSDL

as:
	make -C as
	
test: exp
	./exp -o test.exp $(ARGS)
	hexdump -C test.exp 

run: test.exp
	./bin/emulator -d $^
	
%.exp: %.bin
	./bin/explink -o $@ -c $^

%.bin: %.asm
	./bin/as -o $@ -L $^

clean:
	rm *.bin