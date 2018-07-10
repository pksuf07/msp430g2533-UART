/*
 * MSP430G2533 send (ascii) string command to the unit
 * file name: cmd_uart.c
 * Started from the code in following posting
 * url : http://e2e.ti.com/support/microcontrollers/msp430/f/166/p/287173/1002122
 *
 * Targeting to control Microchip RN2903 board
 *
 * Only basic UART communication is tested!
 */
#include <msp430g2553.h>
#include <string.h> //-- strlen();

#define TXLED       BIT0    //-- LED, not mandatory
#define RXLED       BIT6    //-- LED, not mandatory

#define TXD         BIT2    //-- P1.2 pin
#define RXD         BIT1    //-- P1.1 pin

unsigned char syscmd[]={"sys sleep 120\r\n"};

void uart_init(void);
void uart_send_byte(unsigned char);
void uart_send_string(unsigned char *, size_t);
void replyCommand(void);

#define RXBUFLEN    128      //-- should be word aligned
unsigned char rxlinebuf[RXBUFLEN];      //-- rx line buffer

volatile unsigned int idx=0;            //-- RX buffer index
volatile unsigned int rxlnflag=0;           //-- clear receiver flag

void uart_init( void )
{
    //-- UCA0CTL1 : USCI_A0 control register 1 ------------------------------------
    UCA0CTL1 = UCSWRST;     //-- UCSWRST: Software reset enable (1), USCI logic held in reset state
    UCA0CTL1 |= UCSSEL_2;   //-- bits[7:6] select the SMCLK as the clock source for the UART mode
                            //-- 00 UCLK, 01 ACLK, 10 SMCLK, 11 SMCLK

    UCA0BR0 = 0x08; //-- UCA0BR0 USCI_A0 baud rate control register 0
    UCA0BR1 = 0x00; //-- UCA0BR1 USCI_A0 Baud Rate Control register 1
                    //-- UCBRx[7-0] Clock pre-scaler setting of the baud rate generator
                    //-- The 16-bit value of (UCAxBR0+UCAxBR1x256) forms the pre-scaler value.

    UCA0MCTL = UCBRS2+UCBRS0;   //-- BITCLK Modulation Pattern, Table 15-2, Modulation UCBARSx=5
                    //-- UCBRFx[7-4] fist modulation stage select
                    //-- UCBRSx[3-1] second modulation stage select
                    //-- UCOS16[0]   oversampling mode enabled (0:Disabled, 1:Enabled)

    //-- disable PORT2 and PORT 3 for energy savings -------------------------------
    P2DIR |= 0xFF;  //-- All P2.x outputs
    P2OUT &= 0x00;  //-- All P2.x reset
    P3DIR |= 0xFF;  //-- All P3.x outputs
    P2OUT &= 0x00;  //-- All P3.x reset

    //-- RX and TX pin of PORT1 -----------------------------------------------------
    P1SEL = BIT1 + BIT2;    //-- R1.1 = RXD, P1.2=TXD
    P1SEL2= BIT1 + BIT2;

    UCA0CTL1 &= ~UCSWRST;
}

void uart_send_string(unsigned char * cmdstr, size_t nstr)    //-- at cmd string
{
    unsigned int i, c;

    for(i=0; i < nstr; i++) {
        c=(unsigned char)*cmdstr++;
        uart_send_byte((unsigned char) c);
    }
}

void uart_send_byte( unsigned char data )
{
    P1OUT |= TXLED;                 //-- red LED
    while(!(IFG2 & UCA0TXIFG));
    UCA0TXBUF = data;
    P1OUT &= ~TXLED;                //-- Toggle TXLED
}

void main(void)
{
    //-- unsigned int i=0,j=0;

    WDTCTL = WDTPW + WDTHOLD; // Stop Watchdog timer

    DCOCTL =0;                  //-- select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;      //-- Set DCO
    DCOCTL = CALDCO_1MHZ;

    //-- set up the on-board LEDs
    P1DIR |= RXLED + TXLED;
    P1OUT &= 0x00;

    uart_init();

    UC0IE |= UCA0RXIE; // Enable USCI_A0 RX interrupt

    while( 1 ) {
        uart_send_string((unsigned char*) syscmd, strlen(syscmd));
        __bis_SR_register( LPM3_bits + GIE );
        while(rxlnflag) {
            //-- now the line buffer is filled to interpret
            replyCommand();

            //-- reset the line buffer --------------------
            memset(rxlinebuf, 0, RXBUFLEN);
            rxlinebuf[0]='\0';
            rxlnflag=0; //-- clear the line buffer flag
            idx=0;      //-- clear the line buffer;
        }
    }
}

void replyCommand(void)
{
    //-- now I have one full one from the Microchip RN2903
    //-- for the time being, i will echo back to the chip
    //-- USCI_A0 TX buffer ready?
    uart_send_string((unsigned char *)rxlinebuf, strlen(rxlinebuf));
    //uart_send_byte(0x0D);   //-- CR
    //uart_send_byte(0x0A);   //-- LF
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
    unsigned char c;
    P1OUT |= RXLED;         //-- green LED
    //-- store to the buffer for further processing
    c=UCA0RXBUF;
    rxlinebuf[idx++]=c;
    // uart_send_byte(c);
    if(c == '\n') {   //-- 0x0D, 015, CR; 0x0A, 012, LF;
        rxlnflag=1;
        __bic_SR_register_on_exit(LPM3_bits);
    }
    P1OUT &= ~RXLED;                //-- Toggle RXLED
}
