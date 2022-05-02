#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> // stdio library for sprintf() and sscanf() functions
#define _USE_MATH_DEFINES // for C
#include <math.h> // for getting pi 

void initSPI();
unsigned char spi_io(unsigned char o);
short spi_value(short channel, short voltage);

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
    
    initSPI();
    
    __builtin_enable_interrupts();
    
    int i = 0;
    short value;
    int triangleWave[100]; 
    int sineWave[100];
    
    // initialize triangle wave
    for (i = 0; i < 50; i++) {
        triangleWave[i] = (int) i*1023/50;
    }
    for (i = 0; i < 50; i++) {
        triangleWave[i+50] = (int) (1023 - i*1023/50);
    }
    
    for (i = 0; i < 100; i++) {
        sineWave[i] = (int) ((sin(M_PI/25*i)+1)*(1023/2));
    }    
    
    i = 0;
        
    while (1) {
        // write one byte over SPI1
        LATAbits.LATA0 = 0; // bring CS to low voltage
        value = spi_value(1, triangleWave[i]); 
        //value = spi_value(1,400);
        //value = 0b0111111111111100;
        spi_io(value>>8); // write the first byte
        spi_io(value & 0xff); // write the second byte
        LATAbits.LATA0 = 1; // bring CS to high voltage
        
        LATAbits.LATA0 = 0; // bring CS to low voltage
        value = spi_value(0, sineWave[i]); 
        //value = spi_value(0,300);
        //value = 0b1111111111111100;
        spi_io(value>>8); // write the first byte
        spi_io(value & 0xff); // write the second byte
        LATAbits.LATA0 = 1; // bring CS to high voltage
        
        i++;
        if (i >= 100) {
            i = 0;
        }
        
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < (SYSTEM_FREQUENCY / 200)) { // Each step is delay of 1/100 seconds
        }
    }
}

// initialize SPI1
void initSPI() {
    // Pin B14 has to be SCK1
    // Turn off analog pins
    ANSELA = 0; 
    // Make A0 an output pin for CS
    TRISAbits.TRISA0 = 0;
    LATAbits.LATA0 = 1;
    // Set A1 to SDO1
    RPA1Rbits.RPA1R = 0b0011;
    // Set B5 to SDI1
    SDI1Rbits.SDI1R = 0b0001;

    // setup SPI1
    SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1; // 1000 for 24kHz, 1 for 12MHz; // baud rate to 10 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi 
}


// send a byte via spi and return the response
unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

// create the value that is sent through SPI by choosing the channel and the voltage value
// channel = 1 is writing to output B. 0 is writing to output A.
// voltage should be a value between 0 and 1023 (10-bit number)
short spi_value(short channel, short voltage){
    // create the 16 bit number
    short value = (channel << 15) | (0b111 << 12); // set settings bits
    value = value | (voltage << 2); // add voltage value
    return value;
}