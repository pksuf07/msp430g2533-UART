# msp430g2533-UART
MSP-EXP430G2 / msp430g2553 / UART / ECHO

# Simple UART Echo Test with MSP-EXP430G2 and MSP430G2553
I used Code Composer Studio Version: 8.0.0.00016 by Texas Instruments on Windows 7 Professional. 

* P1.1 TX

* P1.2 RX

* Connection string in short: 115200-8-N-1

You can test the code by connecting UART-2-USB board to the evaluation board (RX->P1.1, TX-P1.2) and putty or CoolTerm program. Again this code is the minimalist, proof of concept, program so I did not have full functionality one might expect/wish to have. Enjoy if you will. I uploaded the picture of my testbed.

* echo_uart.c: Simplely echo the character as it received.

* cmd_uart.c: This is the test code for command-reply transaction through the UART port. Yes, oddly it is in its reverse form because I am developing RN2903 driver. Again it is in it's simplest form. It requires state machine implemented for driving Micochip RN2903 (implemented full stack of LoRaWAN protocol). 


* Note: I am new to MSP430 chip. So, you are always welcome for your generous advices.
