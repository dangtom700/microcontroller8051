#include <reg51.h>
#include <stdio.h>
//#include <display.h>
//#include <control.h>

sbit time_pulse = P0^6;
sbit RA0 = P0^0;
sbit RA1 = P0^1;
sbit RA2 = P0^2;
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

void timer0_delay_100us() {
    TMOD = 0x01;  // Timer 0, Mode 1 (16-bit timer)
    TH0 = 0xFF;   // Load high byte of the preload value
    TL0 = 0xAC;   // Load low byte of the preload value
    TR0 = 1;      // Start Timer 0
    while (TF0 == 0);  // Wait for the timer to overflow
    TR0 = 0;      // Stop Timer 0
    TF0 = 0;      // Clear the Timer 0 overflow flag
}


int On_time_duty_cycle(){
	if (RA0 == 1){
		if (RA1 == 1){
			if (RA2 == 1){
				return 10;// 111B        
			} else {
				return 9;// 110B
			}
		} else {
			if (RA2 == 1){
				return 8;// 101B
			} else {
				return 6;// 100B
			}
		}
	} else {
		if (RA1 == 1){
			if (RA2 == 1){
				return 4;// 011B
			} else {
				return 2;// 010B
			}
		} else {
			if (RA2 == 1){
				return 1;// 001B
			} else {
				return 0;// 000B
			}
		}
	}
	
	return 10;
}

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
        delay_flicker();     // Delay for digit display
        num /= 10;           // Move to the next digit
        sequence <<= 1;      // Shift selector to the next display
    }
}

void main(void) {
	unsigned int i = 0;
	unsigned int RPM = 0;
	int on_time = 0;
	int off_time = 0;
	int on_dutycycle = 0;
	
    while (1) {
		
		//Output to pwn pin
		on_dutycycle = On_time_duty_cycle();		
		// Get ON time and OFF time
		on_time = on_dutycycle;
		off_time = 10 - on_time;
		
		// Activate the duty cycle
		for (i = 0; i < on_time; i++){
			P3 = 0xFF;
			timer0_delay_100us();
		}
		
		for (i = 0; i < off_time; i++){
			P3 = 0x00;
			timer0_delay_100us();
		}
		// Get respective RPM
		/*RPM = read_fan_rpm();
		// Display RPM
		display_num(RPM);*/
    }
}
