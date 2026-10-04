            
      lea STR,r6
      mcro GEN_GC
                inc     r6
                mov     r3, K
                mcroend
GEN_GC:           add     r3, LIST
GEN_MC:           prn     #48
                lea STR ,r6
              
bne     END
                cmp     END, #-6
                bne     %END
                dec     r12
                jmp     %LOOP
END:            stop
.entry r12
 mcro GEN_MC
 mcroend
           ;HELLO everyJew in town
      ;                      HELLO AGAIN     
STR:            .string "abcd"
LIST:           .data   6, -9  
                .data   -100

K:              .data   31