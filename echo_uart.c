#include <msp430g2553.h>
/*
 * A program that waits to receive a certain character from the UART port
 * and then transmit a response to the other device.
 * The communication happens in full duplex at 115200 bps, 8 bits of data
 * with no parity and 1 stop bit.

 */
#define TXLED   BIT0
#define RXLED   BIT6

#define TXD     BIT2
#define RXD     BIT1

int main(void)
{
    WDTCTL = WDTPW + WDTHOLD; // Stop WDT

    //-- set the internal oscillator at 1 MHz
    DCOCTL = 0; // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ; // Set DCO
    DCOCTL = CALDCO_1MHZ;

    //-- UART is only available for USCI_A0 ==> disable the PORT2
    //-- It's always good idea to disable the I/O pins that we don't use
    //-- in order to reduce noise and current consumption.
    P2DIR |= 0xFF;  //-- All P2.x outputs
    P2OUT &= 0x00;  //-- All P2.x reset

    //-- switch to the special function mode
    P1SEL  |= RXD + TXD ;   //-- P1.1 = RXD, P1.2=TXD
    P1SEL2 |= RXD + TXD ;   //-- P1.1 = RXD, P1.2=TXD

    //-- set up the on-board LEDs
    P1DIR |= RXLED + TXLED;
    P1OUT &= 0x00;

    //-- select the SMCLK as the clock source for the UART mode
    UCA0CTL1 |= UCSSEL_2; // SMCLK

    //-- select baud rate
    //-- UCA0BR0 and UCA0BR1 store the integer divider for the SMCLK(1MHz)
    //-- 1MHz/8 = 125000
    //-- We can select 9 because the baud rate will be under 115200bps
    UCA0BR0 = 0x08; // 1MHz 115200
    UCA0BR1 = 0x00; // 1MHz 115200

    //-- UCA0MCTL register for the control of the "modulation"
    //-- it will select the divider between 8 and 9, therefore switching baud rate
    //-- during communication to contain the accumulated error.
    //-- With 8 as divider, we have 125000-115200=9600 (+8.5%) error.
    //-- With 9, we have 115200-111111=4089 (-3.6%) error.
    UCA0MCTL = UCBRS2 + UCBRS0; // Modulation UCBRSx = 5

    UCA0CTL1 &= ~UCSWRST; // **Initialize USCI state machine**

    UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt
    __bis_SR_register(LPM0_bits + GIE); // Enter LPM0 w/ interrupt until Byte RXed

    // while (1){}
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    P1OUT |= RXLED;
    P1OUT |= TXLED;

    //-- USCI_A0 TX buffer ready?
    while(!(IFG2 & UCA0TXIFG));     //-- Poll TXIFG to until set
    UCA0TXBUF = UCA0RXBUF;          //-- TX -> RXed character

    P1OUT &= ~TXLED;                //-- Toggle TXLED
    P1OUT &= ~RXLED;                //-- Toggle RXLED
}
