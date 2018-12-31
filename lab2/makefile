main:my_print.o main.c
	gcc -o main my_print.o main.c -m32 -ggdb3
my_print.o:my_print.asm
	nasm -f elf my_print.asm -o my_print.o -g -F stabs
