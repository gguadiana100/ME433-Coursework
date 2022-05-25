#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> // stdio library for sprintf() and sscanf() functions
#include "ssd1306.h"
#include "font.h"

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

void ssd1306_draw_char(int x, int y, char c); // draw a letter at a position that is 8 pixels tall and 8 pixels wide
void ssd1306_draw_string(int x, int y, char* s); // draw a string starting at a position

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
    
    // setup i2c
    i2c_master_setup();
    
    // setup ssd1306 
    ssd1306_setup();

    __builtin_enable_interrupts();
    
    char message[50]; 
    int counter = 0;
    double FPS;
    int start_time;
    int end_time; 
    
    sprintf(message, "Developed for ME 433"); 
    ssd1306_draw_string(0,0,message);
    sprintf(message, "by Gilberto Guadiana"); 
    ssd1306_draw_string(0,8,message);
    
    while (1) {
        // Heartbeat LED to check if code is running
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 1;
//        ssd1306_drawPixel(50,10,1); // blinking LED
//        ssd1306_update();
        while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
            start_time = _CP0_GET_COUNT();
            sprintf(message, "i:%d", counter); 
            ssd1306_draw_string(0,16,message);
            ssd1306_update();
            end_time = _CP0_GET_COUNT();
            counter = counter + 1;
            FPS = (SYSTEM_FREQUENCY/2)/(end_time - start_time);
            sprintf(message, "Camera FPS is %f", FPS);
            ssd1306_draw_string(0,24,message);
        }
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 0;
//        ssd1306_drawPixel(50,10,0);
//        ssd1306_update();
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
        }
        
//        ssd1306_draw_char(60, 20, counter);
//        ssd1306_update();
//        counter = counter + 1;
//        if (counter > 0x7f) {
//            counter = 0x20;
//        }
        
        
    }
}

// draw a letter at a position that is 8 pixels tall and 5 pixels wide
// the c argument takes in the ASCII code for the char
void ssd1306_draw_char(int x, int y, char c) {
    int letter;
    int mask;
    int current_bit;
    for (int i = 0; i < 5; i++){ // i is the column
        letter = ASCII[c-0x20][i];
        for (int j = 0; j < 8; j++) { // j is the row
            mask = 1 << j; // get the jth bit
            current_bit = (mask & letter) >> j;
            ssd1306_drawPixel(x+i,y+j,current_bit);
        }
    }
}

// draw a string starting at a position
void ssd1306_draw_string(int x, int y, char* s) {
    int index = 0;
    
    while (s[index] != 0) {
        ssd1306_draw_char(x + index * 5, y, s[index]);
        index = index + 1;
    }
}





