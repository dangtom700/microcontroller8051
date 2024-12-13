#include <reg51.h>

#define FOSC 10000000      // Oscillator frequency (10 MHz)
#define TIMER_PRESCALER 12 // 8051 Timer runs at (FOSC / 12)

unsigned char digits[] = {
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

sbit TACH_PIN = P0^4;   // Tachometer signal input
sbit RA0 = P0^0;        // DIP Switch 0
sbit RA1 = P0^1;        // DIP Switch 1
sbit RA2 = P0^2;        // DIP Switch 2
sbit PWM_pin = P3^0;    // PWM Output pin

unsigned int global_display_num = 0; // Number to display on 7-segment
unsigned char digit_index = 0;       // Current digit being displayed (0-3)
unsigned int PWM_on_time = 0;        // On-time for PWM signal
unsigned int PWM_off_time = 0;       // Off-time for PWM signal
bit pwm_state = 0;                   // Current state of the PWM pin (ON/OFF)

// Function to calculate RPM
unsigned int readFanRPM(unsigned char numEdges) {
    unsigned int timeDifference = 0;
    unsigned long rpm = 0; // Use 32-bit for large calculations
    unsigned char edgeCount = 0;

    // Configure Timer 1 in Mode 1 (16-bit timer)
    TMOD &= 0x0F; // Clear Timer 1 bits
    TMOD |= 0x10; // Timer 1 in Mode 1

    TH1 = 0x00;   // Clear high byte
    TL1 = 0x00;   // Clear low byte

    // Ignore the first falling edge
    while (TACH_PIN);  // Wait while pin is high
    while (!TACH_PIN); // Wait while pin is low

    // Start Timer 1
    TR1 = 1;

    // Count the next `numEdges` falling edges
    while (edgeCount < numEdges) {
        while (TACH_PIN);  // Wait while pin is high
        while (!TACH_PIN); // Wait while pin is low
        edgeCount++;
    }

    // Stop Timer 1 after counting the edges
    TR1 = 0;

    // Read Timer 1 value
    timeDifference = (TH1 << 8) | TL1;

    // Convert time difference to microseconds
    timeDifference = (timeDifference * TIMER_PRESCALER) / (FOSC / 1000000); // µs per tick

    // Calculate RPM
    if (timeDifference > 0) {
        rpm = (60000000UL * numEdges) / (timeDifference * 2); // Factor in number of edges
    }

    return (unsigned int)rpm; // Return RPM as an unsigned int
}

void delay_flicker() {
    TMOD &= 0xF0;   // Clear lower 4 bits to configure Timer 0
    TMOD |= 0x02;   // Timer 0 in Mode 2 (8-bit auto-reload)
    TH0 = 0xFA;     // Load reload value for ~1 ms delay (assuming 10 MHz clock)
    TL0 = 0xFA;     // Initial timer value
    TR0 = 1;        // Start Timer 0
    while (TF0 == 0); // Wait for Timer 0 overflow
    TR0 = 0;        // Stop Timer 0
    TF0 = 0;        // Clear overflow flag
}
/*
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
		delay_flicker();
    }
}
*/

// Function to determine on-time duty cycle from inputs
unsigned int On_time_duty_cycle(){
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

// Timer 0 Interrupt Service Routine for 7-segment multiplexing
void timer0_ISR(void) interrupt 1 {
    static unsigned int temp_display_num = 0;
    unsigned char digit;

    if (digit_index == 0) {
        temp_display_num = global_display_num; // Take a snapshot of the number
    }

    digit = temp_display_num % 10;
    temp_display_num /= 10;

    P1 = digits[digit];    // Load segment data
    P2 = 1 << digit_index; // Select corresponding display
    digit_index = (digit_index + 1) % 4;
}

// Timer 1 Interrupt Service Routine for PWM
void timer1_ISR(void) interrupt 3 {
    if (pwm_state) {
        // Turn off the PWM pin
        PWM_pin = 0;
        pwm_state = 0;

        // Load off-time
        TH1 = (65536 - ((PWM_off_time * FOSC) / (TIMER_PRESCALER * 1000000))) >> 8;
        TL1 = (65536 - ((PWM_off_time * FOSC) / (TIMER_PRESCALER * 1000000))) & 0xFF;
    } else {
        // Turn on the PWM pin
        PWM_pin = 1;
        pwm_state = 1;

        // Load on-time
        TH1 = (65536 - ((PWM_on_time * FOSC) / (TIMER_PRESCALER * 1000000))) >> 8;
        TL1 = (65536 - ((PWM_on_time * FOSC) / (TIMER_PRESCALER * 1000000))) & 0xFF;
    }

    TF1 = 0; // Clear the interrupt flag
}

// Main function
void main(void) {
    unsigned int RPM = 0;
    unsigned int duty_cycle = 0;

    // Configure Timer 0 for 7-segment multiplexing (1 ms interrupts)
    TMOD |= 0x02;  // Timer 0 in mode 2 (8-bit auto-reload)
    TH0 = 0xFA;    // Reload value for 1 ms (assuming 10 MHz clock)
    TL0 = 0xFA;
    ET0 = 1;       // Enable Timer 0 interrupt
    TR0 = 1;       // Start Timer 0

    // Configure Timer 1 for PWM
    TMOD |= 0x10;  // Timer 1 in mode 1 (16-bit)
    ET1 = 1;       // Enable Timer 1 interrupt
    TR1 = 1;       // Start Timer 1

    EA = 1;        // Enable global interrupts

    while (1) {
        // Update duty cycle based on DIP switch values
        duty_cycle = On_time_duty_cycle() * 10; // Scale to percentage
        PWM_on_time = duty_cycle * 128 / 100;  // On-time in ticks
        PWM_off_time = (100 - duty_cycle) * 128 / 100; // Off-time in ticks

        // Read RPM and update the global display value
        RPM = readFanRPM(1);            // Calculate RPM from tachometer signal
    }
}
