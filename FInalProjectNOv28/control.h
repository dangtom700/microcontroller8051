#ifndef Control_H
#define Control_H

#include <reg51.h>

sbit RA0 = P0^0;
sbit RA1 = P0^1;
sbit RA2 = P0^2;
sbit RA4 = P0^4;

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

unsigned int get_RPM(int on_time){
	// 0 RPM with 0% duty cycle and 2400 RPM with 100% duty cycle
	// 240 RPM gain for every 10% gain
	return 240 * on_time;
}

#endif // Control_H