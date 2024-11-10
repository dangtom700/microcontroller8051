#ifndef DELAY_H
#define DELAY_H

#include <reg51.h>

void Delay500ms(){
    // Generate a 0.5 second delay for a 10 MHz clock using Timer 0 in 16-bit mode
    unsigned int i;
    for (i = 0; i < 40; i++){   // Loop to generate approximately 0.5 seconds
        // Timer 0 delay calculation for 12-clock cycle 8051 with a 10 MHz clock:
        // 1 machine cycle = 12 / 10 MHz = 1.2 us
        // Max timer count = 65536, thus the timer max duration = 65536 * 1.2 us = 78.432 ms
        // Divide 0.5 seconds by 78.432 ms to get around 6.375 cycles, rounding up to 7
        TMOD = 0x01;   // Timer0 mode 1 (16-bit timer)
        TH0 = 0x3C;    // Load high byte for 50 ms delay
        TL0 = 0xB0;    // Load low byte for 50 ms delay
        TR0 = 1;       // Start Timer0
        while (TF0 == 0);  // Wait for Timer0 overflow flag
        TR0 = 0;       // Stop Timer0
        TF0 = 0;       // Clear overflow flag
    }
}

/*
1. Timer Delay Calculation
With a 10 MHz clock:

Machine cycle time = 12 / 10 MHz = 1.2 탎
Each timer tick takes 1.2 탎.
The 8051's Timer 0 can be set in 16_bit mode (Mode 1), 
which allows counts from 0x0000 to 0xFFFF (65536 ticks).

2. Calculating Timer Reload Values
Using these values, we can calculate the timer reload 
value required to create each delay:

50 ms delay: 
	50,000탎/1.2탎 = 41667 ticks
100 ms delay: 
	100,000탎/1.2탎 = 83333 ticks
200 ms delay: 
	200,000탎/1.2탎 = 166667 ticks
500 ms delay: 
	500,000탎/1.2탎 = 416667 ticks
	
Since the 16_bit timer can count only up to 65536, 
loops have to be used within the delay function to 
achieve these longer delays.
*/
void Timer0_Delay(unsigned int delay_ms) {
    unsigned char i;  // Loop counter
    unsigned int reload_count;

    switch (delay_ms) {
        case 50:
            reload_count = 1;  // Approximately 50 ms with 1 iteration
            TH0 = 0x3C;        // Load Timer0 high byte for 50 ms
            TL0 = 0xB0;        // Load Timer0 low byte for 50 ms
            break;
        case 100:
            reload_count = 2;  // Approximately 100 ms with 2 iterations of 50 ms
            TH0 = 0x3C;
            TL0 = 0xB0;
            break;
        case 200:
            reload_count = 4;  // Approximately 200 ms with 4 iterations of 50 ms
            TH0 = 0x3C;
            TL0 = 0xB0;
            break;
        default: // Default to 500 ms
            reload_count = 10;  // Approximately 500 ms with 10 iterations of 50 ms
            TH0 = 0x3C;
            TL0 = 0xB0;
    }

    TMOD = 0x01; // Set Timer0 to 16-bit mode (Mode 1)

    for (i = 0; i < reload_count; i++) {
        TR0 = 1;         // Start Timer0
        while (TF0 == 0); // Wait for overflow
        TR0 = 0;         // Stop Timer0
        TF0 = 0;         // Clear overflow flag
    }
}
#endif