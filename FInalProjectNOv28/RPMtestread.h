#ifndef RPMtestread_H
#define RPMtestread_H

#include <reg51.h>

sbit TACH_PIN = P0^4;  // Assuming tachometer signal is connected to pin P3.0
#define FOSC 10000000  // Oscillator frequency (10 MHz)
#define TIMER_PRESCALER 12 // 8051 Timer runs at (FOSC / 12)

// Function to calculate RPM from tachometer signal
unsigned int readFanRPM() {
    unsigned int timerCount1 = 0;
    unsigned int timerCount2 = 0;
    unsigned int timeDifference = 0;
    unsigned int rpm = 0;

    // Configure Timer 1
    TMOD = 0x10;  // Timer 1 in mode 1 (16-bit timer)
    TH1 = 0x00;   // Clear high byte
    TL1 = 0x00;   // Clear low byte

    // Wait for the first falling edge
    while (TACH_PIN);  // Wait while pin is high
    while (!TACH_PIN); // Wait while pin is low

    // Start Timer 1
    TR1 = 1;

    // Wait for the second falling edge
    while (TACH_PIN);  // Wait while pin is high
    while (!TACH_PIN); // Wait while pin is low

    // Stop Timer 1
    TR1 = 0;

    // Read Timer 1 value
    timerCount2 = (TH1 << 8) | TL1;

    // Calculate time difference (in microseconds)
    // Timer counts in cycles of (1 / (FOSC / TIMER_PRESCALER))
    timeDifference = timerCount2 * (12.0 / FOSC) * 1000000;

    // Calculate RPM
    // Time difference is the time for one half of a rotation (2 edges per rotation)
    // RPM = 60 / (time for one rotation in seconds)
    rpm = 60000000 / (timeDifference * 2);

    // Return RPM (capped at 2400)
    if (rpm > 2400) {
        rpm = 2400;
    }

    return rpm;
}

#endif
