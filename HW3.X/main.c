#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> // stdio library for sprintf() and sscanf() functions
//#include<C:\Users\Gilberto Guadiana\Desktop\ME 433\AdvancedMechatronicsGithub\HW3.X\uart.h> // helper library in this project directory
//#include<uart.h>

void readUART1(char * string, int maxLength);
void writeUART1(const char * string);

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = FRCPLL // use fast frc oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = OFF // primary osc disabled
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt value
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz fast rc internal oscillator
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 100 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

#define HALF_SECOND_CORE_COUNTS 12000000 // core timer counts for 0.5 seconds at sysclk of 48 MHz
#define SYSTEM_FREQUENCY 48000000 // system frequency of 48 MHz
#define DESIRED_BAUD_RATE 230400 // Baud rate for UART communication

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0; // Set A4 pin to be an output 
    LATAbits.LATA4 = 0; // Set A4 output value to be off
    TRISBbits.TRISB4 = 1; // Set B4 pin to be an input 
    
    RPB7Rbits.RPB7R = 0b0001; // Set RPB7 to U1TX
    U1RXRbits.U1RXR = 0b0001; // Set RPB11 to U2RX
    
    // turn on UART1 without an interrupt
    U1MODEbits.BRGH = 0; // set baud to NU32_DESIRED_BAUD
    U1BRG = ((SYSTEM_FREQUENCY / DESIRED_BAUD_RATE) / 16) - 1;

    // 8 bit, no parity bit, and 1 stop bit (8N1 setup)
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    // configure TX & RX pins as output & input pins
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    // enable the uart
    U1MODEbits.ON = 1;

    __builtin_enable_interrupts();
    
    char m[100];

    // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
    // remember the core timer runs at half the sysclk
    while (1) {
        // if B4 pin is pressed, turn on LED for 0.5 seconds, off, on, and off
        if (PORTBbits.RB4 == 0) {
            _CP0_SET_COUNT(0);
            LATAbits.LATA4 = 1;
            while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
            }
            _CP0_SET_COUNT(0);
            LATAbits.LATA4 = 0;
            while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
            }
            _CP0_SET_COUNT(0);
            LATAbits.LATA4 = 1;
            while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
            }
            _CP0_SET_COUNT(0);
            LATAbits.LATA4 = 0;
            while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
            }
            
            sprintf(m, "Hello!");
            writeUART1(m);
        }
    }
}

// Read from UART1
// block other functions until you get a '\r' or '\n'
// send the pointer to your char array and the number of elements in the array
void readUART1(char * message, int maxLength) {
  char data = 0;
  int complete = 0, num_bytes = 0;
  // loop until you get a '\r' or '\n'
  while (!complete) {
    if (U1STAbits.URXDA) { // if data is available
      data = U1RXREG;      // read the data
      if ((data == '\n') || (data == '\r')) {
        complete = 1;
      } else {
        message[num_bytes] = data;
        ++num_bytes;
        // roll over if the array is too small
        if (num_bytes >= maxLength) {
          num_bytes = 0;
        }
      }
    }
  }
  // end the string
  message[num_bytes] = '\0';
}

// Write a character array using UART1
void writeUART1(const char * string) {
  while (*string != '\0') {
    while (U1STAbits.UTXBF) {
      ; // wait until tx buffer isn't full
    }
    U1TXREG = *string;
    ++string;
  }
}