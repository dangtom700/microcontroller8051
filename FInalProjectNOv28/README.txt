
# About this project

## Introduction

This is the final project for MSE 352 Fall 2024. In this project, we give the micro controller the ability to
control the duty cycle and output the speed in RPM in the circuit board. All of the equipment are provided in
the lab, including power supply to function the circuit and oscilloscope to debug the hardware

## Objectives

- Control the RPM of the fan by controling the duty cycle of the fan
- Measure the rotational speed of the fan by multiple methods to cross check
- Correctly output the speed of the fan (within the last 3 digits)
- Make the fan create a steady response (speed) in respected to given duty cycle

## Instruction

The project is made of two parts: hardware design (circuit assembly) and the software (the core of this project)

### Physical build

The hardware includes a 8051 (40-pins) microcontroller, 4 annode 7-segments LEDs, a MOSFET, a DC brushless fan
motor, an 8-bit DIP switch and 4 BJTs

#### Instruction on how to build

1. The microcontroller feeds 5 VDC into the base of all BJTs.
--- Number display
2. 7-segment displays are wired as 'abcd efg_' to the corresponding output line of RB.[1-7]
3. From RC.[1-4], wire to the collector of all BJTs
4. Corresponding to the hundered, tens and units of the number display, wire the emitter to the corresponding
digit display
--- Fan control
5. PLace an 8-bit DIP switch the directly receives the 5 VDC supply (only use 3 bit for control)
6. Wire all 3 control bits to an empty port
7. Connect the control wire of the fan to the microcontroller

### Digital build

Main programs run in the according instruction:
- Check the encoded signal for RPM
- Decoded the signal and look up the corresponding value
- Give the fan the corresponding duty cycle
- Display the rounded up speed on 4 7-segment displays

## Analysis

The project has 3 major tasks: Communicating the data from and to the fan control, display the RPM and
calculating the delay time for each execution step.

### Reading strategy

From the data sheet of AUB0812L-9X41, the given RPM when 100% duty cycle is 2400, which one can build the
equation to calculate RPM as RPM(ON%) = 24 * ON%. This is the interpolation function from the ideal RPM of
the data sheet

The actual reading RPM of this fan has to be driven by the MOSFET. There is a circuit provided in the datasheet.

### Number interpretation and display

To build the circuit board:
- Three annode 7-segments number displays are wired
- VCC to power to number display is yielded directly from DC power source 
(along with self-customized control circuit)
- The pins are wired as "abcd efg(dot)" corresponding to "0123 4567"

To test the circuit:
- First, turn on all the pins by setting "1111 1111" or "0000 0000"
- Take reference from schematic of the number display or shift 1 bit of "0x01" to find the corresponding segment

		  ____A____
		 /        /
	   F/        /B
	   /____G___/
	  /        /
	E/        /C   _
    /____D___/    |_| DP

### Analyze the time execution

1. Timer Calculation
With a 10 MHz clock:

Machine cycle time = 12 / 10 MHz = 1.2 탎
Each timer tick takes 1.2 탎.
The 8051's Timer 0 can be set in 16_bit mode (Mode 1), 
which allows counts from 0x0000 to 0xFFFF (65536 ticks).

20kHz => 0.00005 sec = 50 \micro s
=> 1% = 0.5 \micro s => [Preload value] = 65536 - [Time delay x Timer increment frequency]
= 65536 - (0.5 * 10MHz) = 60536_{10} = EC78_{16}
=> 10% = 5 \micro s => 65536 - (5 \micro s * 10MHz) = 65486_{10} = FFCE_{16}
==> Flicking rate for number:5/4 \micro s => 1.25 \micro s => 65536 -(1.25 \micro s * 10MHz) = 65523_{10} = 

2. Calculating Timer Reload Values
Using these values, we can calculate the timer reload 
value required to create each delay:

50 ms delay: 
	50,000탎/1.2탎 = 41667 ticks
100 ms delay: 
	100,000탎/1.2탎 = 83333 ticks
200 ms delay: 
	200,000탎/1.2탎 = 166667 ticks
500 ms delay: 
	500,000탎/1.2탎 = 416667 ticks
	
Since the 16_bit timer can count only up to 65536, 
loops have to be used within the delay function to 
achieve these longer delays.

Example:
	50 ms = 1 reload count
	100 ms = 2 reload count
	200 ms = 4 reload count
	500 ms = 10 reload count
	
3. Calculuate TH0 and TLO for delay
The high byte (TH0) and low byte (TL0) registers in an 8051 microcontroller's timer system are used to preload 
values into Timer 0 or Timer 1 before starting the timer. These preloaded values determine the duration of the 
timer delay.

[Timer increment frequency] = [Clock frequency] / [Cycles per machine cycle]
With configuration of 10 MHz and 12 cycles for machine cycles
	Timer increment period = 12 / 1MHz = 1.2 (\micro s)
[Preload value] = 65536 - [Time delay x Timer increment frequency]
With the setting of 100 ms
	Timer count = 65536 - (100 ms x 10 MHz) = -17797
	Take 2's complement = 65536 - 17797 = (47,739)_{10}
	(47,739)_{10} = (0xBA7B)_{16}

## Summary

## Credits

## Reference

Code 27/11/2024 (main loop)
	while (1){
		ON_DUTY_CYLE = On_time_duty_cycle();
		
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
	}