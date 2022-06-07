#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> // stdio library for sprintf() and sscanf() functions
#include "ws2812b.h"

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
    
    // setup ws2812b
    ws2812b_setup();

    __builtin_enable_interrupts();
    //wsColor currentColor = HSBtoRGB(300, 0.1, 0.1);
    wsColor currentColor;
    currentColor.r = 5;
    currentColor.b = 0;
    currentColor.g = 0;
    wsColor colors[5] = {currentColor, currentColor, currentColor, currentColor, currentColor};
    ws2812b_setColor(colors, 5);
    char redness = 0;
    
    while (1) {
        currentColor.r  = redness;
        colors[0] = currentColor;
        // Heartbeat LED to check if code is running
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 1;
        //LATAINV = 0b10000;

        while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
            
        }
        ws2812b_setColor(colors, 3);
        redness = redness + 20;
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 0;
        //LATAINV = 0b10000;

        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
        }
        

        
        
    }
}









