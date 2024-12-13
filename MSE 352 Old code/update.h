#ifndef Update_H
#define Update_H

void ten_percent_PWM(unsigned char sequence){
	TMOD = 0x01;  // Timer 0, Mode 1 (16-bit timer)
    TH0 = 0xEC;   // Load high byte of the preload value
    TL0 = 0x78;   // Load low byte of the preload value
    TR0 = 1;      // Start Timer 0
    while (TF0 == 0){
		P3 = sequence;
	}
    TR0 = 0;      // Stop Timer 0
    TF0 = 0;      // Clear the Timer 0 overflow flag
}

#endif // Update_H