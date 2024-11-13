/*
 * GLCD.h
 *
 * Created: 9/29/2014 1:04:31 AM
 *  Author: Kato
 */ 


#ifndef GLCD_H_
#define GreenLCD_H_

#include "ASCII.h"

// Control I/O port
#define LCD_CONTROL_OUT PORTC
#define LCD_CONTROL_IN PINC
#define LCD_CONTROL_DDR DDRC

// Data I/O port
#define LCD_DATA_OUT PORTA
#define LCD_DATA_IN PINA
#define LCD_DATA_DDR DDRA

// Define LCD control signals
#define LCD_CS1 7
#define LCD_CS2 6
#define LCD_RES 5
#define LCD_RW 4
#define LCD_DI 3
#define LCD_E 2
//#define LCD_LED 7

/*************************************************************/
/*************************************************************/

#define LINE_1 0
#define LINE_2 1
#define LINE_3 2
#define LINE_4 3
#define LINE_5 4
#define LINE_6 5
#define LINE_7 6
#define LINE_8 7

// LCD busy status bit
#define LCD_BUSY 7

// Turn on power to the display, no cursor
#define LCD_ON   0x3F
#define LCD_OFF  0x3E

#define LCD_HEIGHT 64
#define LCD_WIDTH 128

#define PAGE_ADDRESS	0xB8	//First 8 rows of pixels on lcd
#define COLUMN_ADDRESS	0x40	//First column of pixels on lcd
#define Z_ADDRESS		0xC0	//first row of pixels on lcd

/* Global structure to control the graphics LCD display
and also be able to provide controlled cursor movement*/
struct 
{
	unsigned char PageCount;
	unsigned char ColumnPointer;
	unsigned char ColumnData;
	unsigned char ChipSelect;
}LCD;

void LCD_PutData (unsigned char character, unsigned char chipselect);
void LCD_WrtCmd (unsigned char cmd, unsigned char chipselect);
void LCD_BusyWait (unsigned char chipselect);

void LCD_Init (void);
void LCD_ClearPage (unsigned char chipselect);
void LCD_ClearScreen (void);
char LCD_PutChar (unsigned char character);
unsigned char LCD_PutCursor (unsigned char x, unsigned char y);
void LCD_PutColumn (unsigned char character, unsigned char x, unsigned char y);
unsigned char LCD_GetColumn (unsigned char x, unsigned char y);
void LCD_PutString (const char *character);
void LCD_SetPixel (unsigned char x, unsigned char y);
void LCD_ClearPixel (unsigned char x, unsigned char y);
void LCD_ClearLine (unsigned char line);
void LCD_DrawBox (int x1, int x2, int y1, int y2);
void LCD_DrawLine (int x1, int x2, int y1, int y2);


/****************************************************************************/
void LCD_DrawLine (int x1, int x2, int y1, int y2)
{
	int x, y;
	double m;

	x = x1;
	m = (double)(y2 - y1) / (x2 - x1);
	y = (char)((-m*x)+y2);

	while(x < x2) 
	{
		y = (char)((-m*x)+y2);
		LCD_SetPixel (x++, y);
	}
}
/****************************************************************************/

/****************************************************************************/
void LCD_DrawBox (int x1, int x2,int y1, int y2)
{
	int i;
	/*From left to right*/
	for (i = x1; i <= x2; ++i)
	{
		LCD_SetPixel (i, y1);
	}
	/*From up to down*/
	for (i = y1; i<=y2; ++i)
	{
		LCD_SetPixel (x2, i);
	}
	/*From right to left*/
	for (i = x2; i >= x1; --i)
	{
		LCD_SetPixel (i, y2);
	}
	/*From down to up*/
	for (i = y2; i >= y1; --i)
	{
		LCD_SetPixel (x1, i);
	}
}
/****************************************************************************/
/****************************************************************************/
void LCD_ClearPixel (unsigned char x, unsigned char y) 
{
	unsigned char d, q, p, n;

	q = y / 8;
	p = q * 8;
	n = y - p;

	d = LCD_GetColumn (x, q);
	LCD.ChipSelect = LCD_PutCursor (x, q);
	LCD_PutData (~(1<<n)&d, LCD.ChipSelect);
}
/****************************************************************************/

/****************************************************************************/
void LCD_SetPixel (unsigned char x, unsigned char y) 
{
	unsigned char d, q, p, n;

	q = y / 8;
	p = q * 8;
	n = y - p;

	d = LCD_GetColumn (x, q);
	LCD.ChipSelect = LCD_PutCursor (x, q);
	LCD_PutData ((1<<n)|d, LCD.ChipSelect);
}
/****************************************************************************/

/****************************************************************************/
unsigned char LCD_GetColumn (unsigned char x, unsigned char y) 
{

	LCD.ChipSelect = LCD_PutCursor (x, y);
	LCD_BusyWait(LCD.ChipSelect);
	//_delay_us(50);
	LCD_DATA_DDR = 0x00; // Set LCD data port to inputs

	LCD_CONTROL_OUT |= (1<<LCD_RW);
	LCD_CONTROL_OUT &= ~(1<<LCD.ChipSelect);
	// There must be a design flaw in the controller so, do a dummy read
	LCD_CONTROL_OUT |= (1<<LCD_E);
	_delay_ms(10);
	LCD.ColumnData = LCD_DATA_IN; // Dummy read
	LCD_CONTROL_OUT &= ~(1<<LCD_E);
	_delay_ms(10);
	// Do the correct read of the column data
	LCD_CONTROL_OUT |= (1<<LCD_E);
	_delay_ms(10);
	LCD.ColumnData = LCD_DATA_IN;
	LCD_CONTROL_OUT &= ~(1<<LCD_E);
	LCD_CONTROL_OUT |= (1<<LCD.ChipSelect);

	return (LCD.ColumnData);
}
/****************************************************************************/

/****************************************************************************/
void LCD_PutColumn (unsigned char character, unsigned char x, unsigned char y) 
{
	LCD.ChipSelect = LCD_PutCursor (x, y);
	LCD_PutData (character, LCD.ChipSelect);
}
/****************************************************************************/

/****************************************************************************/
unsigned char LCD_PutCursor (unsigned char x, unsigned char y) 
{
	unsigned char ChipSelect;
	
	// ensure that coordinates are legal
	if(x > (LCD_WIDTH -1))
	{
		 x = 0;	
	}
	if(y > (LCD_HEIGHT -1))
	{
		  y = 0;
	}

	LCD.PageCount = y;
	LCD.ColumnPointer = x;
	if (LCD.ColumnPointer < 64)				// Less than half the screen
	{
		ChipSelect = LCD_CS1; // Set display left side
		// Set column address
		LCD_WrtCmd (COLUMN_ADDRESS | LCD.ColumnPointer, ChipSelect);
	}
	else 
	{
		ChipSelect = LCD_CS2; // Set display right side
		// Set column address
		LCD_WrtCmd (COLUMN_ADDRESS | (LCD.ColumnPointer - 64), ChipSelect);
	}
	// Set page address
	LCD_WrtCmd (PAGE_ADDRESS | LCD.PageCount, ChipSelect);
	return (ChipSelect);
}
/****************************************************************************/

/****************************************************************************/
void LCD_PutString (const char *str) 
{
	unsigned char i = 0;

	//LCD.ChipSelect = LCD_PutCursor (x, y);
	while (str[i] != 0) 
	{
		// Check that the next character fits on the current line
		LCD_PutChar (str[i++]);
	}
}
/****************************************************************************/

/****************************************************************************/
char LCD_PutChar (unsigned char ch) 
{
	unsigned char ColumnCounter = 0;

	// Check that the next character fits on the current line
	if ((LCD.ColumnPointer + 6) > (LCD_WIDTH - 1)) 
	{
		LCD.PageCount++;
		LCD.ColumnPointer = 0;
		LCD_CONTROL_OUT |= (1<<LCD_CS2); // Disable the Right side
		LCD_WrtCmd ((PAGE_ADDRESS + LCD.PageCount), LCD_CS1); // Set 'ColumnCounter' page address
		LCD_WrtCmd (COLUMN_ADDRESS, LCD_CS1); // Set 'Y' column address
	}

	// Put the next character out onto the display
	for (ColumnCounter = 0; (ColumnCounter < 6); ColumnCounter++) {
		// Check for the end of the current line
		if (LCD.ColumnPointer > (LCD_WIDTH - 1)) 
		{
			LCD.PageCount++;
			LCD_CONTROL_OUT |= (1<<LCD_CS2); // Disable the Right side
			LCD_WrtCmd ((PAGE_ADDRESS + LCD.PageCount), LCD_CS1); // Set page address
			LCD_WrtCmd (COLUMN_ADDRESS, LCD_CS1); // Set 'Y' column address
			LCD.ColumnPointer = 0;
		}

		// Check for the last page
		if (LCD.PageCount > 7)
		{
			LCD_CONTROL_OUT |= (1<<LCD_CS2); // Disable the Right side
			LCD_WrtCmd (PAGE_ADDRESS, LCD_CS1); // Set page address
			LCD_WrtCmd (COLUMN_ADDRESS, LCD_CS1); // Set 'Y' collunm address
			LCD.PageCount = 0;
			LCD.ColumnPointer = 0;
		}

		// Check to see if the Left line is full, if so, select the Right line
		if (LCD.ColumnPointer == LCD_HEIGHT) 
		{
			LCD_CONTROL_OUT |= (1<<LCD_CS1); // Disable the Left side
			LCD_WrtCmd ((PAGE_ADDRESS + LCD.PageCount), LCD_CS2); // Set page address
			LCD_WrtCmd (COLUMN_ADDRESS, LCD_CS2); // Set 'Y' collunm address
		}

		// Check if Left or Right side should be selected
		if (LCD.ColumnPointer > (LCD_HEIGHT - 1)) 
		{
			LCD.ChipSelect = LCD_CS2;
		}
		else 
		{
			LCD.ChipSelect = LCD_CS1;
		}
		// Send the current column to the display
		LCD_PutData (Ascii[ch][ColumnCounter], LCD.ChipSelect);
		LCD.ColumnPointer++;
	}
	return(-1);
}
/****************************************************************************/

/****************************************************************************/
void LCD_ClearScreen (void) 
{
	LCD_ClearPage (LCD_CS1);
	LCD_ClearPage (LCD_CS2);
}
/****************************************************************************/

/****************************************************************************/
void LCD_ClearPage (unsigned char chipselect) 
{
	unsigned char X, Y;

	X = PAGE_ADDRESS;
	LCD_BusyWait(chipselect);
	//_delay_us(50);
	do {
		LCD_WrtCmd (X, chipselect); // Set 'X' page address
		LCD_WrtCmd (COLUMN_ADDRESS, chipselect); // Set 'Y' column address
		for (Y = 0; (Y < LCD_HEIGHT); Y++) {
			LCD_PutData (0x00, chipselect); // Set data
		}
	} while (++X < PAGE_ADDRESS + 8);
}
/****************************************************************************/

/****************************************************************************/
void LCD_ClearLine (unsigned char line)
{
	LCD_PutCursor(0, line);
	for (int i = 0; i < (LCD_WIDTH/8 + 5); i++)	//Fill line with spaces
	{
		LCD_PutChar(' ');
	}	
}
/****************************************************************************/

/****************************************************************************/
void LCD_PutData (unsigned char character, unsigned char chipselect) 
{
	_delay_us(50);
	LCD_CONTROL_OUT &= ~(1<<LCD_E);
	_delay_us(1);
	LCD_CONTROL_OUT &= ~((1<<chipselect) | (1<<LCD_RW));
	LCD_CONTROL_OUT |= (1<<LCD_DI);
	_delay_us(0.2);
	LCD_CONTROL_OUT |= (1<<LCD_E);
	_delay_us(0.015);
	LCD_DATA_OUT = character;
	_delay_us(0.5);
	LCD_CONTROL_OUT &= ~(1<<LCD_E);
	_delay_us(0.015);
	LCD_CONTROL_OUT |= (1<<chipselect) | (1<<LCD_RW);
	LCD_CONTROL_OUT &= ~(1<<LCD_DI);
	_delay_us(0.5);
	LCD_CONTROL_OUT |= (1<<LCD_E);
}
/****************************************************************************/

/****************************************************************************/
void LCD_WrtCmd (unsigned char cmd, unsigned char chipselect) 
{
	LCD_BusyWait(chipselect);
	//_delay_us(50);
	LCD_CONTROL_OUT &= ~(1<<LCD_E);
	_delay_us(1);
	LCD_CONTROL_OUT &= ~((1<<chipselect) | (1<<LCD_RW) | (1<<LCD_DI));
	_delay_us(0.2);
	LCD_CONTROL_OUT |= (1<<LCD_E);
	_delay_us(0.015);
	LCD_DATA_OUT = cmd;
	_delay_us(0.5);
	LCD_CONTROL_OUT &= ~(1<<LCD_E);
	_delay_us(0.015);
	LCD_CONTROL_OUT |= (1<<chipselect) | (1<<LCD_DI) | (1<<LCD_RW);
	_delay_us(0.5);
	LCD_CONTROL_OUT |= (1<<LCD_E);	
}
/****************************************************************************/

/****************************************************************************/
void LCD_BusyWait (unsigned char chipselect) 
{
	char lcd;
	LCD_DATA_DDR = 0x00; // Set LCD data port to inputs

	LCD_CONTROL_OUT &= ~((1<<LCD_DI) | (1<<chipselect));
	LCD_CONTROL_OUT |= (1<<LCD_RW);
	do {
		LCD_CONTROL_OUT |= (1<<LCD_E);
		_delay_ms(1);
		lcd = LCD_DATA_IN;
		LCD_CONTROL_OUT &= ~(1<<LCD_E);
	} while (lcd & (1<<LCD_BUSY));
	LCD_CONTROL_OUT |= (1<<LCD_DI) | (1<<chipselect);
	LCD_DATA_DDR |= 0xFF; // Set LCD data port to outputs
}
/****************************************************************************/

/****************************************************************************/
void LCD_Init (void) 
{
	_delay_ms(50);
	// Initialize the AVR controller I/O
	LCD_DATA_DDR = 0xFF; // Set LCD_DATA_DDR to all outputs
	LCD_DATA_OUT = 0x00; // Set LCD_DATA_OUT to logic low
	LCD_CONTROL_DDR = 0xFC; // Set LCD_CONTROL_DDR to all outputs
	LCD_CONTROL_OUT |= (1<<LCD_DI) | (1<<LCD_CS1) | (1<<LCD_CS2)|(1<<LCD_RES);
	LCD_WrtCmd (LCD_ON, LCD_CS1); // Power up the Left side controller of the display
	LCD_WrtCmd (LCD_ON, LCD_CS2); // Power up the Right side controller of the display
	// Wait 1 millisecond for the display to stabilize
	_delay_ms(1);
}
/****************************************************************************/

#endif /* GLCD_H_ */