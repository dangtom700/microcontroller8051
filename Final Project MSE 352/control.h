#ifndef Control_H
#define Control_H

#define CRYSTAL_FREQUENCY 10000000  // 10 MHz crystal
#define TIMER_PRESCALER 12          // 8051 timer frequency is Fosc / 12
#define TICKS_PER_SECOND (CRYSTAL_FREQUENCY / TIMER_PRESCALER)
#define FAN_MAX_RPM 2400

#include <reg51.h>

sbit time_pulse = P0^4;

void timer0_start() {
    TMOD = 0x01; // Set Timer 0 in Mode 1 (16-bit timer)
    TH0 = 0x00;  // Clear the high byte
    TL0 = 0x00;  // Clear the low byte
    TR0 = 1;     // Start Timer 0
}

unsigned int timer0_read() {
    unsigned int timer_value;
    timer_value = (TH0 << 8) | TL0; // Combine high and low bytes
    return timer_value;
}

void timer0_stop() {
    TR0 = 0; // Stop Timer 0
}

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

void pwm_output(){
	int on_time = 0;
	int off_time = 0;
	int on_dutycycle = 0;
	int i = 0;
	
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
		
}

unsigned int measure_time_between_rising_edges() {
    unsigned int time_ticks = 0;
    unsigned char overflow_count = 0;

    TMOD = 0x01;  // Timer0 in mode 1 (16-bit timer)
    TH0 = 0x00;   // Clear Timer0 high byte
    TL0 = 0x00;   // Clear Timer0 low byte
    TF0 = 0;      // Clear Timer0 overflow flag

    // Wait for the first rising edge
    while (TACH_PIN);  // Wait until the pin goes low
    while (!TACH_PIN);   // Wait until the pin goes high
    while (TACH_PIN);  // Wait until the pin goes low again

    TR0 = 1;  // Start Timer0

    // Wait for the second rising edge
    while (!TACH_PIN);  // Wait until the pin goes high
    TR0 = 0;            // Stop Timer0

    // Read the timer value
    time_ticks = (TH0 << 8) | TL0;

    // Convert to total time in ticks (account for overflows if needed)
    time_ticks += (overflow_count * 65536);

    return time_ticks;
}

unsigned int calculate_rpm(unsigned int time_ticks) {
    // Calculate time in seconds per rotation
    float time_per_rotation = (float)time_ticks / TICKS_PER_SECOND;

    // Convert to rotations per minute (RPM)
    unsigned int rpm = (unsigned int)((1.0 / time_per_rotation) * 60.0);

    // Cap RPM at the maximum expected value
    if (rpm > FAN_MAX_RPM) {
        rpm = FAN_MAX_RPM;
    }

    return rpm;
}

unsigned int read_fan_rpm() {
    unsigned int time_ticks = measure_time_between_rising_edges();
    return calculate_rpm(time_ticks);
}

unsigned int get_RPM(int on_time){
	// 0 RPM with 0% duty cycle and 2400 RPM with 100% duty cycle
	// 240 RPM gain for every 10% gain
	return 240 * on_time;
}
unsigned int get_RPM_time_pulse() {
    unsigned int start_time, end_time, tick_count;
    
    // Wait for the first rising edge of the pulse
    while (time_pulse == 0); // Wait for time_pulse to go HIGH
    start_time = timer0_read(); // Capture start time
    
    // Wait for the next rising edge of the pulse
    while (time_pulse == 1); // Wait for time_pulse to go LOW
    while (time_pulse == 0); // Wait for time_pulse to go HIGH again
    end_time = timer0_read(); // Capture end time

    // Calculate the tick count
    if (end_time >= start_time) {
        tick_count = end_time - start_time; // No overflow
    } else {
        tick_count = (65536 - start_time) + end_time; // Account for overflow
    }

    // Ensure no division by zero
    if (tick_count == 0) {
        return 0; // Return 0 RPM if the pulse interval is invalid
    }
	return 60000000 / (1.2 * tick_count);
}

#endif // Control_H