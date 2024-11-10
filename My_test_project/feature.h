#ifndef FEATURE_H
#define FEATURE_H

#include <reg51.h>
#include <delay.h>

void singlePortAct(bit input_port, bit output_port){
    // If port2_0 receives an ON signal, activate the LED on port1_1
    if (input_port == 1){    // Check if the input port is high (ON signal)
        output_port = 1;      // Turn on LED on P1.1
    } else {
        output_port = 0;      // Turn off LED
    }
}

void invert_chaser_pattern(unsigned int cycle){
    unsigned char pattern = 0xFE;  // Initial pattern: 1111 1110B
	unsigned int i = 0;

    for (i = 0; i < cycle; i++){
        P1 = pattern;       // Output the current pattern to Port 1
        Delay500ms();       // Wait for 0.5 seconds
        pattern = (pattern << 1) | (pattern >> 7);  // Rotate left
    }
}

/* INSTRUCTION
To build the circuit board:
- Two annode 7 segments number displays are wired
- VCC to power to number display is yielded directly from DC power source 
(along with self-customized control circuit)
- The pins are wired as "abcd efg(dot)" corresponding to "0123 4567"

To test the circuit:
- First, turn on all the pins by setting "1111 1111" or "0000 0000"
- Take reference from schematic of the number display or manual testing to 
adjust the display to show a number of interest

		  ____A____
		 /        /
	   F/        /B
	   /____G___/
	  /        /
	E/        /C   _
    /____D___/    |_| DP
*/

// Array to store segment patterns for numbers 0-9
unsigned char digits[] = { 
	0x01,  // n0: 0000 0001 in binary,
	0x61,  // n1: 0110 0001 in binary,
	0xDB,  // n2: 1101 1011 in binary,
	0xF3,  // n3: 1111 0011 in binary,
	0x67,  // n4: 0110 0111 in binary,
	0xB7,  // n5: 1011 0111 in binary,
	0x7B,  // n6: 0111 1011 in binary,
	0xE1,  // n7: 1110 0001 in binary,
	0xFF,  // n8: 1111 1111 in binary,
	0xE7   // n9: 1110 0111 in binary,
};

void number_muxer(void){
	unsigned char unit_count = 0;
    unsigned char ten_count = 0;
    unsigned char isCountUp = 1;  // 1 for counting up, 0 for counting down

    while(1){
        P1 = digits[unit_count];  // Display units on Port 1
        P2 = digits[ten_count];   // Display tens on Port 2
        
        if (isCountUp) {   // Count up logic
            unit_count++;  // Increment the unit count
            if (unit_count > 9) { 	 // If units overflow
                unit_count = 0;    	 // Reset units
                ten_count++;       	 // Increment tens
                if (ten_count > 9) { // If tens overflow
                    ten_count = 0;   // Reset tens
                }
            }
        } else {  // Count down logic
            if (unit_count == 0) {    // If units underflow
                unit_count = 9;       // Set units to max
                if (ten_count == 0) { // If tens underflow
                    ten_count = 9;    // Set tens to max
                } else {
                    ten_count--;      // Decrement tens
                }
            } else {
                unit_count--;         // Decrement units
            }
        }

        Delay500ms(); // Delay to see the result on display
    }
}

/*
Binary Counter Display with LEDs and DIP Switches
	- Circuit: Connect each of the 8 DIP switch outputs to 8 LEDs 
	through BJTs to act as current drivers.
	- Application: This circuit can serve as a binary display or 
	simple input-output test board. When a switch is toggled on, 
	the corresponding LED lights up, showing the binary representation.
	- Use Case: This setup is ideal for learning about binary 
	counting and logic, where each switch represents a binary_bit.
	
Instruction:
- Set 8 bits dipswitch with pull down resistors to port 1 as follow:
	Slot [1234 5678] is pinned correspondingly into port 1 line [0123 4567]
- Set 8 LEDs to port 2 to output the signal

Process:
- As one line in the sending port a 1 digital signal, the corresponding
receiving port line output the signal to turn ON / off the LED
*/

void binary_decimal_converter(void) {
	unsigned char encode = 0x00;
	while (1){
		encode = ~P1;
		/*
		Depending on the circuit topology, the LEDs many already receive
		an ON signal, so when the micro controller sends the 1 digtial
		signal. The LEDs turn OFF as a result of current cancellation.
		
		If the circuit is set to positive by default
			- NOT (complement) every bits in the sending port
		If the circuit is set to negative by default
			- Deliver as it is in the output port
		*/
		switch (encode) {
			case 0x00:
				encode = digits[0];
				break;
			case 0x01:
				encode = digits[1];
				break;
			case 0x02:
				encode = digits[2];
				break;
			case 0x03:
				encode = digits[3];
				break;
			case 0x04:
				encode = digits[4];
				break;
			case 0x05:
				encode = digits[5];
				break;
			case 0x06:
				encode = digits[6];
				break;
			case 0x07:
				encode = digits[7];
				break;
			case 0x08:
				encode = digits[8];
				break;
			case 0x09:
				encode = digits[9];
				break;
		}
	}
}

/*
Digital Lock System Using DIP Switch and LEDs
	- Circuit: Use the DIP switch as a binary input for a password. 
	The microcontroller checks the switch settings (password) and 
	lights up an LED if correct.
	- Application: A simple digital lock where a specific combination
	on the DIP switch turns on a “lock opened” LED.
	- Use Case: Good for understanding digital input and logical 
	comparisons. Could be used in real-world applications for asmall
	safe or container.
	
Instruction:
- Set 8 bits dipswitch with pull down resistors to port 1 as follow:
	Slot [1234 5678] is pinned correspondingly into port 1 line [0123 4567]
- Set 8 LEDs to port 2 to output the signal

Process:
- User tries to unlock with different combination of bit_switches
- If the port matches the password, then all LEDs light on as correct
- The lock locks itself after certain amount of time and reset the password
itself for the next unlock

Result is a simple digital lock with an 8_bit dipswitch
*/

void digital_lock(void){
	unsigned int counter = 1;
	unsigned char password = 0x63;
	const unsigned int threshold = 5;
	
	while (1){
		P2 = 0x00;
		
		if (P1 == password){
			P2 = 0xFF;
			Delay500ms();
			counter++;
		}
		
		if (counter > threshold){
			P1 = P1 ^ 0x34 | (~P1 ^ 0x56);
			password = (password << 1) | (password >> 7);
			counter = 0;
		}
	}
}

/*
Binary converter to decimal in 0-9

Instruction:
- 
*/

/* LED patterns display
Instruction:
- Set 8 LEDs to port 1 for display

There are many patterns to take reference from:

1. Chaser pattern: One blink at a time, increment and 
in a loop

*/
void chaser_pattern(unsigned int cycle){
	unsigned char sequence = 0x01;// 0000 0001 equivalent
	unsigned int i = 0;
	for (i = 0; i < cycle; i++){
		P1 = sequence;
		// Shift 1_bit to the left
		sequence = (sequence << 1) | (sequence >> 7);
		Timer0_Delay(200);
	}
}
/*

2. Bllinking pattern: Light up all the same time and
dim down all the same time

*/
void blinking_pattern(unsigned int cycle) {
	unsigned int i = 0;
	for (i = 0; i < cycle; i++){
		P1 = 0xFF;
		Timer0_Delay(500);
		P1 = 0x00;
	}
}
/*

3. ALternating pattern: Light up every second LED
and vice versa

*/
void alternating_pattern(unsigned int cycle) {
	unsigned char sequence = 0xAA;// 1010 1010 equivalent
	unsigned int i = 0;
	
	for (i = 0; i < cycle; i++){
		P1 = sequence;
		// Shift 1_bit to the right
		sequence = (sequence >> 1) | (sequence << 7);
		Timer0_Delay(200);
	}
}
/*

4. Knight rider pattern: Light up in sequence and go
back and forth, in medium pace

*/
void knight_rider_pattern(unsigned int cycle) {
	unsigned char sequence = 0xFE;// 0000 0001 equivalent
	unsigned char isReverse = 0;
	unsigned int counter = 0;
	unsigned int i = 0;
	
	for (i = 0; i < cycle; i++){
		P1 = sequence;	// Output the port signal
		Timer0_Delay(500);
		
		if (counter > 8){
			isReverse = ~isReverse; // Complement condition
			counter = isReverse ? 6 : 0;
		}
		
		if (isReverse == 1){
			// Shift 1_bit to the right
			sequence = (sequence >> 1) | (sequence << 7);
			counter--;
		} else {
			// Shift 1_bit to the left
			sequence = (sequence << 1) | (sequence >> 7);
			counter++;
		}
	}
}
/*

5. Wave pattern: Light up in sequence, back and forth,
fast pace

*/
void wave_pattern(unsigned int cycle) {
	unsigned char sequence = 0xFE;// 0000 0001 equivalent
	unsigned char isReverse = 0;
	unsigned int i = 0;
	unsigned int counter = 0;
	
	for (i = 0; i < cycle; i++){
		P1 = sequence;	// Output the port signal
		Timer0_Delay(500);
		
		
		if (counter > 8){
			isReverse = ~isReverse; // Complement condition
		}
		
		if (isReverse == 1){
			// Shift 1_bit to the right
			sequence = (sequence >> 1) | (sequence << 7);
			counter--;
		} else {
			// Shift 1_bit to the left
			sequence = (sequence << 1) | (sequence >> 7);
			counter++;
		}
	}
}
/*

6. Ping pong pattern: Light up the far_end pair inward
to the center, and outward again

*/
void ping_pong_pattern(unsigned int cycle) {
	unsigned char pattern = 0x81;  // Initial pattern: 1000 0001
    unsigned char direction = 0;   // Direction: 1 for left, 0 for right
	unsigned int i = 0;
	
	for (i = 0; i < cycle; i++) {
		P1 = pattern;
		Timer0_Delay(500);
		
		if (pattern == 0x18 || pattern == 0x81){
			direction = ~ direction; // Reverse condition
		}
		if (direction){ // Shift 1_bit left
			pattern = (pattern << 1) | (pattern >> 7);
		} else { // Shift 1_bit right
			pattern = (pattern >> 1) | (pattern << 7);
		}
	}
}
#endif