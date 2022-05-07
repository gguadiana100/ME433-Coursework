#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> // stdio library for sprintf() and sscanf() functions
#include<C:\Users\Gilberto Guadiana\Desktop\ME 433\AdvancedMechatronicsGithub\HW6.X\i2c_master_noint.h>

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

void setPin(unsigned char address, unsigned char reg, unsigned char value); // I2C set pin to value
unsigned char readPin(unsigned char writeAddress, unsigned char readAddress, unsigned char reg); // I2C read from pin

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
    // TRISBbits.TRISB4 = 1; // Set B4 pin to be an input 
    
    // setup i2c
    i2c_master_setup();

    __builtin_enable_interrupts();

    unsigned char writeDeviceAddress = 0b01000000; // A2, A1, A0 are set to ground
    unsigned char readDeviceAddress = 0b01000001; // A2, A1, A0 are set to ground
    unsigned char GP0Value;
    
    // set GP0 to be an input pin and GP7 to be output pin by setting IODIR register
    setPin(writeDeviceAddress, 0x00, 0b0000001);
    // set GP7 output to be low by setting OLAT register
    setPin(writeDeviceAddress, 0x0A, 0b00000000);
    
    while (1) {
        // Read GPIO register for pin values
        GP0Value = readPin(writeDeviceAddress, readDeviceAddress, 0x09);
        // Select out only the value of GP0
        GP0Value = GP0Value & 0b1;
        if (GP0Value == 0) { // check if button is pushed
            //setPin(writeDeviceAddress, 0x0A, 0xff); // set pin GP7 to be high
            setPin(writeDeviceAddress, 0x0A, 0b10000000);
        }
        else { // button is not pushed
            //setPin(writeDeviceAddress, 0x0A, 0x0); // set pin GP7 to be low
            setPin(writeDeviceAddress, 0x0A, 0);
        }
        
        // Heartbeat LED to check if code is running
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 1;
        while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
        }
        _CP0_SET_COUNT(0);
        LATAbits.LATA4 = 0;
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < HALF_SECOND_CORE_COUNTS) {
        }
        
    }
}

// I2C set pin to value
void setPin(unsigned char address, unsigned char reg, unsigned char value){
    i2c_master_start(); // send start signal
    i2c_master_send(address); // send address of device with a write bit
    i2c_master_send(reg); // send register to set
    i2c_master_send(value); // send value to set
    i2c_master_stop(); // send stop signal
    return;
}

// I2C read from pin
unsigned char readPin(unsigned char writeAddress, unsigned char readAddress, unsigned char reg){
    unsigned char value;
    i2c_master_start(); // send start signal
    i2c_master_send(writeAddress); // send address of device with a write bit
    i2c_master_send(reg); // send register to read from
    i2c_master_restart(); // send restart signal
    i2c_master_send(readAddress); // send address of device with a read bit
    value = i2c_master_recv(); // receive a byte
    i2c_master_ack(1); // send an acknowledge signal
    i2c_master_stop(); // send stop signal
    return value;
}




