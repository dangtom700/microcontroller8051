#include <reg51.h>
#include <stdio.h>

/*
The project includes:
- Reading RPM setting from an 8-bit DIP switch.
- Processing the corresponding PWM signal to drive the fan motor.
- Using Timer 0 in Mode 2 to count tachometer edges during the PWM period.
- Displaying the RPM value on a 7-segment display.
- Dynamically adjusting the PWM duty cycle to match the desired RPM.
*/

// System inputs and outputs
sbit TACH_PIN = P0^4;    // Tachometer input pin
sbit RA0 = P0^0;         // DIP switch bit 0
sbit RA1 = P0^1;         // DIP switch bit 1
sbit RA2 = P0^2;         // DIP switch bit 2
sbit PWM_pin = P3^0;     // PWM output pin

// Global variables
unsigned char digits[] = { // Array to store segment patterns for numbers 0-9
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

volatile unsigned int edge_count = 0;    // Falling edge count for tachometer
volatile unsigned int global_display_num = 0; // Number to display on 7-segment
unsigned int target_RPM = 0;             // Target RPM from DIP switch
unsigned int current_RPM = 0;            // Measured RPM
unsigned int PWM_on_time = 50;           // On time in 탎 (initially 100% duty cycle)
unsigned int PWM_period = 50;            // PWM period in 탎 (20 kHz frequency)

// Function prototypes
unsigned int calculate_RPM();            // Calculate RPM based on edge count
int On_time_duty_cycle();                // Get duty cycle setting from DIP switch
void display_num(unsigned int num);      // Display number on 7-segment
void setup_timers();                     // Configure and start timers

// Timer 1 ISR: Handles PWM generation and 7-segment display multiplexing
void timer1_ISR() interrupt 3 {
    static unsigned int PWM_counter = 0;

    // PWM signal generation
    if (PWM_counter < PWM_on_time) {
        PWM_pin = 1;  // Turn PWM pin ON
    } else {
        PWM_pin = 0;  // Turn PWM pin OFF
    }

    // Increment PWM counter
    PWM_counter += 1; // Timer 1 tick duration = 1 탎
    if (PWM_counter >= PWM_period) {
        PWM_counter = 0; // Reset counter at the end of the period
    }

    // Update 7-segment display
    display_num(global_display_num);
}

// Timer 0 ISR: Count tachometer falling edges
void timer0_ISR() interrupt 1 {
    if (!TACH_PIN) {      // If falling edge detected
        edge_count++;     // Increment edge count
    }
}

// Main loop
void main(void) {
    setup_timers();  // Initialize Timer 0 and Timer 1

    while (1) {
        // Step 1: Read target RPM from DIP switch
        target_RPM = On_time_duty_cycle() * 240; // Scale DIP switch value to RPM (max 2400 RPM)

        // Step 2: Calculate RPM based on edge count
        current_RPM = calculate_RPM();
        global_display_num = current_RPM; // Update number to display on 7-segment

        // Step 3: Adjust PWM duty cycle based on target RPM
        if (current_RPM < target_RPM - (target_RPM * 0.05)) { // If current RPM is 5% below target
            PWM_on_time += 1;  // Increase duty cycle (step size = 1 탎)
        } else if (current_RPM > target_RPM + (target_RPM * 0.05)) { // If current RPM is 5% above target
            if (PWM_on_time > 1) { // Ensure on-time does not go below 0
                PWM_on_time -= 1;  // Decrease duty cycle (step size = 1 탎)
            }
        }

        // Ensure PWM_on_time stays within bounds
        if (PWM_on_time > PWM_period) PWM_on_time = PWM_period; // Max is 100%
        if (PWM_on_time < 1) PWM_on_time = 1;                   // Min is ~2% duty cycle
    }
}

// Function to calculate RPM from edge count
unsigned int calculate_RPM() {
    unsigned int RPM = 0;
    unsigned int edges = edge_count;    // Copy edge count to avoid ISR interference
    edge_count = 0;                     // Reset edge count for the next cycle

    // Calculate RPM (assuming 2 edges per revolution)
    RPM = (edges * 60000) / (PWM_period * 2); // RPM = (edges * 60 s) / (period * 2 edges/rev)
    return RPM;
}

// Function to get duty cycle setting from DIP switch
int On_time_duty_cycle() {
    if (RA0 == 1) {
        if (RA1 == 1) {
            if (RA2 == 1) return 10;   // 100%
            else return 9;             // 90%
        } else {
            if (RA2 == 1) return 8;    // 80%
            else return 6;             // 60%
        }
    } else {
        if (RA1 == 1) {
            if (RA2 == 1) return 4;    // 40%
            else return 2;             // 20%
        } else {
            if (RA2 == 1) return 1;    // 10%
            else return 0;             // 0%
        }
    }
	return 0;
}

// Function to display a number on a 7-segment display
void display_num(unsigned int num) {
    static unsigned char digit_index = 0;
    unsigned char digit = 0;

    // Extract the current digit to display
    digit = (num / (unsigned int)(1 << (4 * digit_index))) % 10;

    // Update 7-segment display
    P1 = digits[digit];    // Send digit pattern to P1
    P2 = 1 << digit_index; // Activate the corresponding display line

    // Move to the next digit
    digit_index = (digit_index + 1) % 4;  // Cycle through digits 0-3
}

// Function to configure and start timers
void setup_timers() {
    // Configure Timer 0 for edge counting (Mode 2)
    TMOD |= 0x02;    // Timer 0 in Mode 2 (8-bit auto-reload)
    TH0 = 0x00;      // Initial value for counting edges
    TL0 = 0x00;
    ET0 = 1;         // Enable Timer 0 interrupt
    TR0 = 1;         // Start Timer 0

    // Configure Timer 1 for PWM and 7-segment multiplexing
    TMOD |= 0x01;    // Timer 1 in Mode 1 (16-bit timer)
    TH1 = 0xFF;      // Reload value for 1 탎 ticks
    TL1 = 0x00;
    ET1 = 1;         // Enable Timer 1 interrupt
    TR1 = 1;         // Start Timer 1

    // Enable global interrupts
    EA = 1;
}
