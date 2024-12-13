#include <reg51.h>
#include <stdio.h>
#include <display.h>
#include <control.h>
#include <RPMtestread.h>
#include <RPMtestreadnew.h>

void main(void) {
	int on_time = 0;
	int off_time = 0;
	unsigned int iteration = 0;
	unsigned int RPM = 0;
	int ON_DUTY_CYLE = 0;
	
    while (1) {
		// Get ON time duty cycle
		ON_DUTY_CYLE = On_time_duty_cycle();
		// Get respective RPM
		//RPM = get_RPM(ON_DUTY_CYLE);
		
		if (on_time <= 0 && off_time <= 0){
			on_time = ON_DUTY_CYLE;
			off_time = 10 - on_time;
		}
		
		// Activate the duty cycle
		if (on_time > 0){
			P3 = 1;
			//timer0_delay_100us();
			on_time --;
		}
		
		if (off_time > 0){
			P3 = 0;
			//timer0_delay_100us();
			off_time --;
		}
		
		// Display RPM
		RPM = readFanRPM();
		display_num(RPM);
    }
}
