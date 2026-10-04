            mcro GEN_MC
      lea STR,r6
                inc     r6
                mov     r3, K
                mcroend
MAIN:           add     r3, LIST, STR
LOOP:           prn     #48
            GEN_MC
                lea STR ,r6
                inc     r6
                mov     r3,   K
                sub     r1, r4
                bne     
                cmp     END, #-6
                bne     %END
                dec     r12
                jmp     #1
END:            stop
.entry r12
 
      ;  HELLO      
STR:            .string "abcd"
LIST:           .data  , 6, ,-9,  
                .data   -100

K:              .data   31