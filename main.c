/*
 * File:   main.c
 * Author: slava
 *
 * Created on May 8, 2016, 7:17 PM
 */
#define _XTAL_FREQ 31000

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)

#include <xc.h>

typedef void (*Handler)(void);

void waitForLight(void);
void waitForKeyRelease(void);

#define MIN_TIME 240

Handler handler = waitForLight;

unsigned char sec_count;
unsigned char bp;

void Timer1_on(void) {
    //Timer1 Registers Prescaler= 1 - TMR1 Preset = 34285 - Freq = 1.00 Hz - Period = 1.000032 seconds
    T1CONbits.T1CKPS1 = 0; // bits 5-4  Prescaler Rate Select bits
    T1CONbits.T1CKPS0 = 0; // bit 4
    T1CONbits.TMR1CS = 0; // bit 1 Timer1 Clock Source Select bit...0 = Internal clock (FOSC/4)

    TMR1H = 229; // preset for timer1 MSB register
    TMR1L = 046; // preset for timer1 LSB register

    // Interrupt Registers
    PIR1bits.TMR1IF = 0; // clear timer1 interrupt flag TMR1IF
    PIE1bits.TMR1IE = 1; // enable Timer1 interrupts
    INTCONbits.PEIE = 1; // bit6 Peripheral Interrupt Enable bit...1 = Enables all unmasked peripheral interrupts

    T1CONbits.TMR1ON = 1; // bit 0 enables timer
}

void Timer1_off(void) {
    // Interrupt Registers
    PIR1bits.TMR1IF = 0; // clear timer1 interrupt flag TMR1IF
    PIE1bits.TMR1IE = 0; // enable Timer1 interrupts
    INTCONbits.PEIE = 0; // bit6 Peripheral Interrupt Enable bit...1 = Enables all unmasked peripheral interrupts

    T1CONbits.TMR1ON = 0; // bit 0 enables timer

}

void blink(void) {
    GP4 = 0; //LED on
    __delay_ms(1);
    GP4 = 1; //LED on
}

void pSSSt(void) {
    unsigned char i;

    // blink with led
    for (i = 0; i < 3; i++) {
        blink();
        __delay_ms(200);
    }
    // push button for 700 mS
    GP5 = 1; //LED on
    __delay_ms(700);
    GP5 = 0; //LED on

}

void timerOff(void) {

    if (sec_count >= MIN_TIME) pSSSt();

    handler = waitForLight;
    Timer1_off();
}

void waitForKeyRelease(void) {
    unsigned char keyBuf;

    keyBuf = 0xff;

    while (keyBuf) {
        keyBuf <<= 1;
        if (GP3 == 1) keyBuf++;
        __delay_us(10);
    }

}

void keyPressed(void) {
    pSSSt();
    waitForKeyRelease();
    sec_count = 0;
}

unsigned char lightBuf;

void mainTimer(void) {

    if (bp == 1) {
        bp = 0;
        if (sec_count < 4) blink();

        lightBuf <<= 1;
        if (GP2 == 1) lightBuf++;
        if ((lightBuf & 0x07) == 0) {
            timerOff();
        }
    }

    if (GP3 == 1) {
        keyPressed();
    }
}

void lightOn(void) {
    handler = mainTimer;

    sec_count = 0;
    bp = 0;
    lightBuf = 0x01;

    Timer1_on();
}

void waitForLight(void) {
    SLEEP();
    lightOn();
}

void interrupt tc_int(void) { // interrupt function
    if (INTCONbits.INTF) { // if timer flag is set & interrupt enabled
        INTCONbits.INTF = 0; // clear the interrupt flag
    }
    // Timer1 Interrupt
    if (PIR1bits.TMR1IF == 1) { // timer 1 interrupt flag
        PIR1bits.TMR1IF = 0; // interrupt must be cleared by software
        TMR1H = 229; // preset for timer1 MSB register
        TMR1L = 046; // preset for timer1 LSB register
        bp = 1;
        if (sec_count < MIN_TIME) sec_count++;
    }
}

void main(void) {
    CMCON0 = 7;
    ANSELbits.ANS = 0;
    ADCON0bits.ADON = 0;

    OSCCONbits.IRCF0 = 0; //set OSCCON IRCF bits to select OSC frequency=31kHz
    OSCCONbits.IRCF1 = 0;
    OSCCONbits.IRCF2 = 0;

    GPIO = 0x10;
    TRISIO = 0x0C; //all pins as Output PIN except 2 and 3

    OPTION_REGbits.INTEDG = 1; // falling edge trigger the interrupt

    INTCONbits.INTF = 0; // clear the interrupt flag
    INTCONbits.INTE = 1; // enable the external interrupt
    INTCONbits.GIE = 1; // Global interrupt enable

    if (GP2 == 1) lightOn();

    while (1) {
        handler();
    }

    return;
}
