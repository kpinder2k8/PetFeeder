/*
 * Keypad.h
 * 
 * Created: 10/7/2014 5:17:48 PM
 *  Author: Darius Van
 *  Retrieves value from Keypad. Function to call is "GET_KEYPAD_NUMBER()". Returns Char value corressponding to key
 */ 


#ifndef KEYPAD_H_
#define KEYPAD_H_

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

//**************************************************************************************************************
//Define Port Keypad is connected to. 
#define KeyPadPortOut PORTD
#define KeyPadPinDirection DDRD
#define KeyPadInput PIND
//**************************************************************************************************************




//**************************************************************************************************************
// Definitions for individual switches for Keypad Polling
 
#define COL1 0xEF					//Note: Keypad Values are right adjusted. Bit7 of Port NC
#define COL2 0xDF					//Pin Arrangement: Top to Bot Row = Pin 6, 5, 4, 3
#define COL3 0xBF					//				   Left to Right Col = Pin 2, 1, 0
									//Lower 4 bits ensure pull up resistors are active when each column is outputted 

#define zero		0x57
#define one			0x6E			
#define two			0x5E
#define three		0x3E
#define four		0x6D
#define five		0x5D
#define six			0x3D
#define seven		0x6B
#define eight		0x5B
#define nine		0x3B
#define star		0x67
#define pound		0x37

//**************************************************************************************************************
//**************************************************************************************************************

volatile uint8_t key;

//Function Declaration
char GET_KEYPAD_NUMBER();

//**************************************************************************************************************
//**************************************************************************************************************
//Note: Scans Columns of Keypad and returns a character corresponding to respective character pushed. 

//**************************************************************************************************************
//**************************************************************************************************************

char GET_KEYPAD_NUMBER()
{
	KeyPadPortOut = COL1;				//Scan Column 1
	
	while(1)
	{
		if((KeyPadPortOut | 0x80) == COL1)			//Scan Column 1, Pull Up Resisters Active
		{
			if((KeyPadInput & 0x7F) == one)
			{
				key = 0x01;
				return '1';
			}
			else if((KeyPadInput & 0x7F) == four)
			{
				key = 0x04;
				return '4';
			}
			else if((KeyPadInput & 0x7F) == seven)
			{
				key = 0x07;
				return '7';
			}
			else if ((KeyPadInput & 0x7F) == star)
			{
				key = 0x40;
				return '*';
			}
			
			KeyPadPortOut = COL2;			
		}
		else if((KeyPadPortOut | 0x80) == COL2)		//Scan Column 2
		{
			if((KeyPadInput & 0x7F) == two)
			{
				key = 0x02;
				return '2';
			}
			else if((KeyPadInput & 0x7F) == five)
			{
				key = 0x05;
				return '5';
			}
			else if((KeyPadInput & 0x7F) == eight)
			{
				key = 0x08;
				return '8';
			}
			else if ((KeyPadInput & 0x7F) == zero)
			{
				key = 0x00;
				return '0';
			}
			
			KeyPadPortOut = COL3;
		}
		else if((KeyPadPortOut | 0x80) == COL3)			//Scan Column 3
		{
			if((KeyPadInput & 0x7F) == three)
			{
				key = 0x03;
				return '3';
			}
			else if((KeyPadInput & 0x7F) == six)
			{
				key = 0x06;
				return '6';
			}
			else if((KeyPadInput & 0x7F) == nine)
			{
				key = 0x09;
				return '9';
			}
			else if ((KeyPadInput & 0x7F) == pound)
			{
				key = 0x60;
				return '#';
			}
			
			KeyPadPortOut = COL1;
		}
	}
}



#endif /* KEYPAD_H_ */