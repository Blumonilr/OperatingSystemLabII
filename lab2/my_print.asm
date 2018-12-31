section .data
blue: db 1Bh,'[34;1m',0;
bluelen: equ $ -blue;
defaultc: db 1Bh,'[0m';
defaultclen: equ $ -defaultc;


section .text

global my_print

my_print:

mov eax,[esp+12]
cmp eax,1
je printBlue
jmp print

printBlue:
mov eax,4
mov ebx,1
mov ecx,blue
mov edx,bluelen
int 80h
jmp print

print:
mov eax,4
mov ebx,1
mov ecx,[esp+4]
mov edx,[esp+8]
int 80h

mov eax,4
mov ebx,1
mov ecx,defaultc
mov edx,defaultclen
int 80h
ret
