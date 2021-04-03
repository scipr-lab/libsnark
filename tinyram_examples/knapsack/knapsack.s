; TinyRAM V=2.000 M=vn W=16 K=16
;;; knapsack
;;; primary input: TARGET a_1 a_2 ... a_k
;;; auxiliary input: b_k b_{k-1} ... b_1
;;; answer = TARGET - (a_1 * b_1 + ... + a_k * b_k) (or answer 1 if b_i's are not all 0/1)
store.w 0, r0                    ; 0:
mov r0, 32768                    ; 4:
read r1, 0                       ; 8:
cjmp 28                          ; 12:
add r0, r0, 14                   ; 16:
store.w r0, r1                   ; 20:
jmp 8                            ; 24:
store.w 32768, r0                ; 28: end of prelude
_loop: cmpe r0, 32770            ; 32:
cjmp _bail                       ; 36: bail out if end of input
load.w r1, r0                    ; 40: read [r0] into r1
sub r0, r0, 2                    ; 44: r0 -= 2
read r2, 1                       ; 48: read auxiliary input in r2 (note that pairs are processed in reverse order)
cmpa r2, 1                       ; 52: abort if r2 > 1
cjmp _fail                       ; 56:
mull r3, r1, r2                  ; 60: r3 = [r0] * aux
add r4, r4, r3                   ; 64: r4 += [r0] * aux
jmp _loop                        ; 68: another iteration
_bail: load.w r5, r0             ; 72: r5 = [r0] = [32770]
sub r4, r4, r5                   ; 76: check if cumulative sum is [32770]
answer r4                        ; 80:
_fail: answer 1                  ; 84:
