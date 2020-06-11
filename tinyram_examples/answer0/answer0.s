; TinyRAM V=2.000 M=vn W=16 K=16
store.w 0, r0
mov r0, 32768
read r1, 0
cjmp 28
add r0, r0, 2
store.w r0, r1
jmp 12
store.w 32768, r0
answer 0
