; TinyRAM V=2.000 M=vn W=16 K=16
;;; knapsack with indirect addressing
;;; primary input: TARGET a_1 a_2 ... a_{k}
;;; auxiliary input: idx_1 idx_2 ... idx_l in sorted order
;;; answer = TARGET - (a_{idx_1} + ... + a_{idx_l})
;;; 
;;; register usage:
;;; r0 -- last memory location used (set by prelude, unchanged by us)
;;; r1 -- last idx_i read
;;; r2 -- current idx_i read
;;; r3 -- address for a_{idx_i}
;;; r4 -- value of a_{idx_i}
;;; r5 -- running sum
;;; r6 -- target value, target value - running sum
store.w 0, r0                    ; 0:
mov r0, 32768                    ; 4:
read r1, 0                       ; 8:
cjmp 28                          ; 12:
add r0, r0, 2                    ; 16:
store.w r0, r1                   ; 20:
jmp 8                            ; 24:
store.w 32768, r0                ; 28: end of prelude
_loop:  read r2, 1               ; 32: read idx_i
cjmp _done                       ; 36: if tape ended go and perform the final comparison
cmpae r1, r2                     ; 40: if r1 >= r2 the auxiliary input is incorrect and we answer 1
cjmp _fail                       ; 44:
mov r1, r2                       ; 48: set r1 := r2 for the next loop iteration
mull r3, r2, 2                   ; 52: compute r3 = 32770 + idx_i * 2
cjmp _fail                       ; 56: (fail if overflow)
add r3, r3, 32770                ; 60:
cjmp _fail                       ; 64: (fail if overflow)
load.w r4, r3                    ; 68: load [a_{idx_i}]. note that this could be past end but those values are 0 and thus don't affect knapsack
add r5, r5, r4                   ; 72: update the running sum
jmp _loop                        ; 76: perform the next iteration
_done: load.w r6, 32770          ; 80: r6 = [32770] = target
sub r6, r6, r5                   ; 84: now r6 = target - running sum
answer r6                        ; 88: accepts (i.e. "answer 0") only if if target=running sum
_fail: answer 1                  ; 92: failure case for double reads
