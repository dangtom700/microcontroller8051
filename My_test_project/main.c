#include <reg51.h>
#include <stdio.h>
#include <delay.h>
#include <feature.h>

void main (void){
	alternating_pattern(10);
	blinking_pattern(10);
	chaser_pattern(10);
	invert_chaser_pattern(10);
	knight_rider_pattern(10);
	ping_pong_pattern(10);
	wave_pattern(10);
	
	digital_lock();
	singlePortAct(P1^0, P2^0);
	binary_decimal_converter();
	number_muxer();
}