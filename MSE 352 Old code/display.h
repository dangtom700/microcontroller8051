#ifndef Display_H
#define Display_H

#include <reg51.h>

unsigned char digits[] = {// Array to store segment patterns for numbers 0-9
	0xC0, // HEX for 0
    0xF9, // HEX for 1
    0xA4, // HEX for 2
    0xB0, // HEX for 3
    0x99, // HEX for 4
    0x92, // HEX for 5
    0x82, // HEX for 6
    0xF8, // HEX for 7
    0x80, // HEX for 8
    0x90, // HEX for 9
	0xFF  // HEX for [Blank]
};

void delay_flicker() {
    TMOD = 0x01;    // Timer 0 in Mode 1 (16_bit timer)
    TH0 = 0xF6;     // Load high byte of the preload value (0xF6E4)
    TL0 = 0xF4;     // Load low byte of the preload value
    TR0 = 1;        // Start Timer 0
    while (TF0 == 0); // Wait for Timer 0 overflow
    TR0 = 0;        // Stop Timer 0
    TF0 = 0;        // Clear overflow flag
}


// Function to display a number on a 7-segment display
void display_num(unsigned int num) {
    unsigned char sequence = 0x01;  // Start with hundreds display selector
    unsigned char holder = 0;       // To store individual digits
    unsigned char leading_zero = 1; // Flag for leading zeros
    unsigned int i = 0;             // Initialize the iteration

    for (i = 0; i < 4; i++) {
        holder = num % 10; // Extract the least significant digit

        if (holder == 0 && num < 10) {
            P1 = digits[10]; // Display blank for remaining digits
        } else {
            P1 = digits[holder];
        }

        P2 = sequence;       // Activate the current display
        //delay_flicker();     // Delay for digit display
        num /= 10;           // Move to the next digit
        sequence <<= 1;      // Shift selector to the next display
    }
}

#endif // Display_H