/*
 * PetFeeder_UserInterface.c
 *
 * Created: 11/25/2014 9:35:47 AM
 *  Author: Kato
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ASCII.h" 
#include "GLCD.h"
#include "RTC.h"
#include "Keypad.h"

#define	ENTER	0
#define CANCEL	1
#define RESET	2
#define	FEED	3
#define RF		4



void set_hour(void);
void RTC_set(void);
void time_display(void);
void set_CurrentTime(void);
void feed_time1(int line);
void feed_time2(int line);
void PinChange_InterruptSetup(void);
void Disable_Unusedapps(void);
void alarm_compare(uint8_t min1, uint8_t hr1, uint8_t min1_2, uint8_t hr1_2, uint8_t min2, uint8_t hr2, uint8_t min2_2, uint8_t hr2_2);


//Need to figure out when to enable global interrupt

int main(void)
{
	LCD_Init();
	/*Set up keypad pin directions*/
	KeyPadPinDirection = 0x70;
	/*Set up pin direction for LCD backlight and turn it on*/
	DDRD |= (1<<PIND7);
	PORTD |= (1<<PIND7);
	/*Set interface buttons, and RF  as inputs*/
	DDRB &= ~((1<<ENTER)|(1<<CANCEL)|(1<<RESET)|(1<<FEED)|(1<<RF));
	/*Set feed and user-done transmit pins as output*/
	DDRB |= (1<<PORTB5)|(1<<PORTB6);
	/*Set initial port pins low*/
	PORTB &= ~((1<<PORTB5)|(PORTB6));
	PinChange_InterruptSetup();
	/*Disable clocks to unused modules to save power*/
	Disable_Unusedapps();			
		
	/*16-bits mean that counter ticks up to 65535. With a 1MHz clock, we need at least 
	1,000,000/65,535 = 15.26 prescaler. Closest available is 64. 
	1,000,000/64 = 15625 ticks per second*/
	TCCR1B = (1<<CS11)|(1<<CS10)|(1<<WGM12);
	/*Set MAX for timer counter to reach one second*/
	OCR1A = 15625-1;
	
	/*Enable global interrupts*/
	sei();
	
	///////////////////////////////////
	//		Programming Pet Tag		 //
	///////////////////////////////////
		
	LCD_PutCursor(0,LINE_3);
	LCD_PutString("Please Program a Tag");
		
	/* Wait here until user programs tag*/
	while(bit_is_clear(PINB,RF));
		
	/*Restore original flag value and notify of success*/
	LCD_ClearLine(LINE_3);
	LCD_PutCursor(0,LINE_3);
	LCD_PutString("Program Successful");
	_delay_ms(1000);
	LCD_ClearLine(LINE_3);
	
	///////////////////////////////////
	//		Time Initialization		 //
	///////////////////////////////////	
	ReEnter_Time:
	/*Allow user to enter the current time*/	
	//LCD_DrawBox(28,92,0,10);

	set_CurrentTime();
	
	/*Poll buttons to ensure user entered the correct time, if not, then redo*/
	while (1)
	{	if (bit_is_set(PINB,ENTER))					//Enter Button
		{
			LCD_ClearLine(LINE_3);
			LCD_ClearLine(LINE_4);
			//ENABLE TIMER INTERRUPT HERE, MAYBE????
			break;			
		}
		else if (bit_is_set(PINB,CANCEL))			//Cancel Button
		{
			/*Return to set the current time if error was made*/
			goto ReEnter_Time;
		}
	}
	
	//SET TIME HERE, MAYBE????
	/*Set time on DS1307 RTC chip*/
	RTC_set();
	TIMSK1 |= 1<<OCIE1A;	
		

	
// 	/*Set time on DS1307 RTC chip*/
// 	RTC_set();
	
	///////////////////////////////////
	//		Setting Feeding Times	 //
	///////////////////////////////////	
	ReEnter_Feeding:
	LCD_ClearScreen();
	LCD_PutCursor(30,LINE_3);
	LCD_PutString("Feed Times:");
	LCD_PutCursor(30,LINE_5);
	LCD_PutString("1 --:--AM");
	LCD_PutCursor(30,LINE_6);
	LCD_PutString("2 --:--PM");
	LCD_PutCursor(30,LINE_7);
	LCD_PutString("Enter time");
	///////////////////////////////////
	feed_time1(LINE_5);
	feed_time2(LINE_6);
	///////////////////////////////////
	LCD_ClearLine(LINE_7);
	LCD_PutCursor(30,LINE_7);
	LCD_PutString("Correct?");
	LCD_PutCursor(30,LINE_8);
	LCD_PutString("Press Y/N");
	
	while(1)
	{
		if (bit_is_set(PINB,ENTER))
		{
			//TIMSK1 |= 1<<OCIE1A;
			LCD_ClearLine(LINE_7);
			LCD_ClearLine(LINE_8);
			/*Send signal to second board to notify that user is done*/
			PORTB |= (1<<PORTB6);
			_delay_ms(250);
			PORTB &= ~(1<<PORTB6);
			break;
		}
		else if (bit_is_set(PINB,CANCEL))
		{
			goto ReEnter_Feeding;
		}
	}
	
    while(1)
    {
		if (bit_is_set(PINB,RESET))
		{
			TIMSK1 &= ~(1<<OCIE1A);
			//break;
			goto ReEnter_Feeding;
		}
    }
	
	
}

void RTC_set(void)
{
	/*initialize seconds to 00*/
	rtc_write(0x00,0x00);		
	/*setting minutes*/
	rtc_write(0x01,(((min1<<4))|min2));
	/*setting hours in 12-hr mode. 0x40 for AM, 0x60 for PM ...second term sets hour*/
	rtc_write(0x02,(ampm | ((hr1<<4)|hr2)));													
	/*Enable output compare timer interrupt, and start RTC clock*/
	TIMSK1 |= 1<<OCIE1A;
}

void time_display(void)
{
	LCD_PutCursor(30,LINE_1);
	rtc_read(0x02,&data);            //reading hour from DS1307
	display_hour(data);              //Calling display fn

	LCD_PutChar(':');
	
	rtc_read(0x01,&data);            //reading minute from Ds1307
	display_min_sec(data);                   //Calling display fn

	LCD_PutChar(':');

	rtc_read(0x00,&data);            //reading seconds from Ds1307
	display_min_sec(data);                   //Calling display fn
	
	LCD_PutChar(' ');
	
	rtc_read(0x02,&data);
	display_ampm(data);
}

void feed_time1(int line)
{
	LCD_PutCursor(0,line);
	info = GET_KEYPAD_NUMBER();
	alarm_hr1 = key;
	_delay_ms(500);
	LCD_PutCursor(42,line);
	LCD_PutChar(info);
	info = GET_KEYPAD_NUMBER();
	_delay_ms(500);
	alarm_hr2 = key;
	
	LCD_PutCursor(48,line);
	LCD_PutChar(info);
	////////////////////////////////////
	LCD_PutCursor(0,line);
	info = GET_KEYPAD_NUMBER();
	_delay_ms(500);
	alarm_min1 = key;
	LCD_PutCursor(60,line);
	LCD_PutChar(info);
	info = GET_KEYPAD_NUMBER();
	alarm_min2 = key;
	_delay_ms(500);
	LCD_PutCursor(66,line);
	LCD_PutChar(info);
}

void feed_time2(int line)
{
	LCD_PutCursor(0,line);
	info = GET_KEYPAD_NUMBER();
	alarm2_hr1 = key;
	_delay_ms(500);
	LCD_PutCursor(42,line);
	LCD_PutChar(info);
	info = GET_KEYPAD_NUMBER();
	_delay_ms(500);
	alarm2_hr2 = key;
	
	LCD_PutCursor(48,line);
	LCD_PutChar(info);
	////////////////////////////////////
	LCD_PutCursor(0,line);
	info = GET_KEYPAD_NUMBER();
	_delay_ms(500);
	alarm2_min1 = key;
	LCD_PutCursor(60,line);
	LCD_PutChar(info);
	info = GET_KEYPAD_NUMBER();
	alarm2_min2 = key;
	_delay_ms(500);
	LCD_PutCursor(66,line);
	LCD_PutChar(info);
}

/*Routine that allows user to set the */
void set_CurrentTime(void)
{
	LCD_ClearScreen();
	LCD_PutCursor(30,LINE_1);
	LCD_PutString("--:--:--__");
	///////////////////////////////////
	LCD_ClearLine(LINE_3);
	LCD_PutCursor(30,LINE_3);
	LCD_PutString("Enter Hour");
	data = GET_KEYPAD_NUMBER();
	hr1 = key;
	_delay_ms(500);
	LCD_PutCursor(30,LINE_1);
	LCD_PutChar(data);
	data = GET_KEYPAD_NUMBER();
	hr2 = key;
	_delay_ms(500);
	LCD_PutCursor(36,LINE_1);
	LCD_PutChar(data);
	////////////////////////////////////
	LCD_ClearLine(LINE_3);
	LCD_PutCursor(30,LINE_3);
	LCD_PutString("Enter Min");
	data = GET_KEYPAD_NUMBER();
	min1 = key;
	_delay_ms(500);
	LCD_PutCursor(48,LINE_1);
	LCD_PutChar(data);
	data = GET_KEYPAD_NUMBER();
	min2 = key;
	_delay_ms(500);
	LCD_PutCursor(54,LINE_1);
	LCD_PutChar(data);
	LCD_PutCursor(66,LINE_1);
	LCD_PutString("00");
	////////////////////////////////////
	LCD_ClearLine(LINE_3);
	LCD_PutCursor(30,LINE_3);
	LCD_PutString("Set AM/PM");
	LCD_ClearLine(LINE_4);
	LCD_PutCursor(30,LINE_4);
	LCD_PutString("*=AM, #=PM");
	info = GET_KEYPAD_NUMBER();
	ampm = key;
	_delay_ms(500);
	if (info == '*')
	{
		LCD_PutCursor(78,LINE_1);
		LCD_PutString("AM");
	}
	else if (info == '#')
	{
		LCD_PutCursor(78,LINE_1);
		LCD_PutString("PM");
	}
	/////////////////////////////////////
	LCD_ClearLine(LINE_3);
	LCD_PutCursor(30,LINE_3);
	LCD_PutString("Correct?");
	LCD_ClearLine(LINE_4);
	LCD_PutCursor(30,LINE_4);
	LCD_PutString("Press Y/N");
}

void alarm_compare(uint8_t min1, uint8_t hr1, uint8_t min1_2, uint8_t hr1_2, uint8_t min2, uint8_t hr2, uint8_t min2_2, uint8_t hr2_2)
{
	uint8_t data1, data2, data3, data4, temp;
	/*Takes first and second hour digits from keypad and stores it into the alarm hour for comparing*/
	alarm_hr = ((hr1<<4)|hr1_2);
	/*Takes first and second minute digits from keypad and stores it into the alarm minute for comparing*/
	alarm_min = ((min1<<4)|min1_2);
	alarm2_hr = ((hr2<<4)|hr2_2);
	alarm2_min = ((min2<<4)|min2_2);
	
	rtc_read(0x00,&temp);			//reading seconds from DS1307
	data4 = temp & 0x7F;			
	rtc_read(0x02,&data1);          //reading hour from DS1307
	data3 = data1 & 0x20;			//mask am/pm bit
	data1 = data1 & 0x1F;			//mask hours
		
	rtc_read(0x01,&data2);			//reading minute from DS1307
	

	/*If current time is feeding time, then send signal to turn motor*/
	if (data1 == alarm_hr && data2 == alarm_min && data3 == 0x00 && data4 == 0x00 )
	{
		PORTB |= (1<<PINB5);
		_delay_ms(500);
		PORTB &= ~(1<<PINB5);
	}
	else if (data1 == alarm2_hr && data2 == alarm2_min && data3 == 0x20 && data4 == 0x00 )
	{
		PORTB |= (1<<PINB5);
		_delay_ms(500);
		PORTB &= ~(1<<PINB5);
	}
}

void PinChange_InterruptSetup(void)
{
	/*PCINT 15:8 Interrupt enable*/
	PCICR |= (1<<PCIE1);
	/*Interrupt enable on PB3*/								
	PCMSK1 |= (1<<PCINT11);
}

void Disable_Unusedapps(void)
{
	/*Disable clocks to USART1/0 and ADC*/
	PRR0 |= (1<<PRUSART1) | (1<<PRUSART0) | (1<<PRADC); 
}


/*Pin change Interrupt Service Routine */
/*When the Feed button is pressed, interrupt sets Feed pin to one
  to alert 2nd uP to dispense food*/
ISR(PCINT1_vect)
{	
	/*If feed button was pressed, then send signal to turn motor*/
	if (bit_is_set(PINB,FEED))
	{
		PORTB |= (1<< PINB5);
		_delay_ms(500);
		PORTB &= ~(1<<PINB5);
	} 
	/*If reset was pressed, then erase everything and re-start*/
// 	else if (bit_is_set(PINB,RESET))
// 	{
// 		RF_receivedflag = 1;
// 	}
	/*Clear interrupt flag*/
	PCIFR |= (1<<PCIF1);
}

/*Timer Counter 1 Compare Match A Interrupt Service Routine/Interrupt Handler*/
/*Will check RTC chip every second to update time to LCD and check to see if
  Current time matches the feeding time*/
ISR(TIMER1_COMPA_vect)
{
	time_display();
	alarm_compare(alarm_min1, alarm_hr1, alarm_min2, alarm_hr2, alarm2_min1, alarm2_hr1, alarm2_min2, alarm2_hr2);
}

