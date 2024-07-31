  /*
  
  License GPLv3

  Based on https://github.com/jaredkrinke/avr-n64-usb/blob/master/avr-n64.h 
  

  */

  .equ PINB,0x03
  .equ DDRB,0x04
  .equ PIND,0x09
  .equ DDRD,0x0A
  .equ PORTF,0x11


  ;; remove the L literal from F_CPU
  .equ F_CPU_INT, 0
  .irpc digit, F_CPU
  .ifnc \digit,L
  .equ F_CPU_INT, F_CPU_INT*10 + \digit
  .endif
  .endr



  .section .data
  .extern recvbuffer
  .extern sendbuffer
  .section .text



  .macro _delay_1u shorten=0
  .rept (F_CPU_INT/1000000)-\shorten
  nop
  .endr
  .endm

  .macro _delay_2u shorten=0
  .rept (2*F_CPU_INT/1000000)-\shorten
  nop
  .endr
  .endm

  .macro _delay_1u5 shorten=0
  .rept ( (F_CPU_INT +F_CPU_INT/2) /1000000)-\shorten
  nop
  .endr
  .endm


  .macro  _n64_write_bit p, b, shorten
  //Drive low for 1 tick
  sbi \p, \b
  _delay_1u 3

  // Drive data value for 2 ticks
  or r18,r20
  out \p,r18
  _delay_2u 1

  //Drive high for 1 tick
  cbi \p, \b
  _delay_1u \shorten

  .endm

  .macro  _n64_write_stop_bit p, b, shorten
  sbi \p, \b
  _delay_1u 2
  cbi \p, \b
  _delay_2u \shorten
  .endm

  .macro _n64_send_message p, b, shorten
  rjmp checkIfLast\@
  writenextbyte\@:
  _n64_write_bit \p, \b, 7
  checkIfLast\@:
  ld r18,Z+               ;2
  cpse r18,r19            ;2+1
  rjmp writenextbyte\@    ;2
  writestop\@:
  nop
  _n64_write_stop_bit \p, \b, \shorten
  .endm



  .macro _64_read_message p,b
  readbit\@:
  clr r18                 ;1
  loops\@:inc r18         ;1
  breq endread\@          ;1
  sbic \p,\b
  rjmp loops\@
  _delay_1u5 3
  in r18,\p
  andi r18, (1<<\b)
  st X+,r18
  ld r18,X
  loopc\@:inc r18
  breq endread\@
  sbis \p,\b
  rjmp loopc\@
  rjmp readbit\@
  endread\@:
  ldi r18,0xFF
  st X,r18
  .endm


  .macro _n64_transaction wp,rp,b
  push XH
  push XL
  push ZH
  push ZL
  ldi ZH,hi8(sendbuffer)
  ldi ZL,lo8(sendbuffer)
  ldi XL,hi8(recvbuffer)
  ldi XL,lo8(recvbuffer)
  in r20,\wp
  ldi r19,0xFF
  cli                     ; disable interrupts
  _n64_send_message \wp, \b,4
  _64_read_message \rp, \b
  sei                     ;enable interrupts
  pop ZL
  pop ZH
  pop XL
  pop XH
  .endm

  .global transaction_30          
  .global transaction_31


transaction_30:
  _n64_transaction DDRD,PIND,0
  ret

transaction_31:
  _n64_transaction DDRD,PIND,1
  ret
