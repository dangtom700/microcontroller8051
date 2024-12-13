#ifndef RPMtestreadnew_H
#define RPMtestreadnew_H
#include <reg51.h>

sbit TACH_PIN1 = P0^4; // Assuming tachometer signal is connected to pin P3.0
#define FOSC 10000000  // Oscillator frequency (10 MHz)
#define TIMER_PRESCALER 12 // 8051 Timer runs at (FOSC / 12)
#define MAX_TIMER_COUNT 65536  // 16-bit timer max value

unsigned long overflowCount = 0;  // To track timer overflows

// Timer 1 overflow interrupt service routine
void timer1Overflow() interrupt 3 {
    overflowCount++;
}

// Function to calculate RPM from tachometer signal
unsigned int readFanRPMnew() {
    unsigned long timerCount1 = 0;
    unsigned long timerCount2 = 0;
    unsigned long timeDifference = 0;
    unsigned int rpm = 0;

    // Configure Timer 1
    TMOD = 0x10;  // Timer 1 in mode 1 (16-bit timer)
    TH1 = 0x00;   // Clear high byte
    TL1 = 0x00;   // Clear low byte
    overflowCount = 0;  // Reset overflow counter
    ET1 = 1;      // Enable Timer 1 overflow interrupt
    EA = 1;       // Enable global interrupts

    // Wait for the first falling edge
    while (TACH_PIN1);  // Wait while pin is high
    while (!TACH_PIN1); // Wait while pin is low

    // Start Timer 1
    TR1 = 1;

    // Wait for the second falling edge
    while (TACH_PIN1);  // Wait while pin is high
    while (!TACH_PIN1); // Wait while pin is low

    // Stop Timer 1
    TR1 = 0;

    // Calculate timer count, including overflow
    timerCount2 = ((unsigned long)overflowCount * MAX_TIMER_COUNT) + (TH1 << 8) | TL1;

    // Convert timer count to time difference in microseconds
    timeDifference = timerCount2 * (12.0 / FOSC) * 1000000;

    // Calculate RPM
    rpm = 60000000 / (timeDifference * 2);

    // Return RPM (capped at 2400)
    if (rpm > 2400) {
        rpm = 2400;
    }

    return rpm;
}
#endif