Overview

Software emulation of UART by CCGZKR from EEWORLD
Based on KEIL MDK 5.18

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect to a TTL serial interface with a 3-wire cable
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board and run.
4.  Send '0' to the board and the LED on GPIOB0 status will
    toggle, other characters have no effects.

Note
================
This is just a nearly half-done work, and the GPS code
is under developing. The final target is to achieve a 
GPS logger, one emulated UART port connect to GPS module,
and the on-board one connect to PC for uploading the logged
data. For this part is not yet done, the GPS code is not
added here.

Later the PCB and a shell will be developed in the future.

When flashing the compiled ROM, the following information
is given, and the program has not go through any dynamic 
verification, I am trying to fix this.

Erase Done.
Programming Done.
Contents mismatch at: 0000001CH  (Flash=00H  Required=5FH) !
Contents mismatch at: 0000001DH  (Flash=00H  Required=73H) !
Contents mismatch at: 0000001EH  (Flash=00H  Required=FEH) !
Contents mismatch at: 0000001FH  (Flash=00H  Required=DFH) !
Verify Failed!
Error: Flash Download failed  -  "Cortex-M0+"
Flash Load finished at 23:16:19