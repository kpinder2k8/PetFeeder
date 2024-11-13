/*
 * RTC.h
 *
 * Created: 10/19/2014 11:38:16 PM
 *  Author: Kato
 */ 


#ifndef RTC_H_
#define RTC_H_

void rtc_read(unsigned char addr,unsigned char *data);
void rtc_write(unsigned char addr,unsigned char data);
void display_min_sec(unsigned char temp);
void display_hour(unsigned char temp);
void display_ampm(unsigned char temp);
void display_month(unsigned char temp);
void display_day(unsigned char temp);
char *num2str(char number);
char dec2bcd(char num);
char bcd2dec(char num);

// char *day[]=		{"Sun","Mon","Tue","Wed","Thurs","Fri","Sat"};
// char *month[]=		{"Jan","Feb","Mar","Apr","May","June","July","Aug","Sept","Oct","Nov","Dec"};
char *hour[]=		{"0","1","2","3","4","5","6","7","8","9","10","11","12"};
char *min_sec[]=	{"00","01","02","03","04","05","06","07","08","09","10","11","12",
					"13","14","15","16","17","18","19","20","21","22",
					"23","24","25","26","27","28","29","30","31","32",
					"33","34","35","36","37","38","39","40","41","42",
					"43","44","45","46","47","48","49","50","51","52",
					"53","54","55","56","57","58","59"};
char *am_pm[]=		{"AM","PM"};
uint8_t data;
uint8_t hr1=0, hr2=0, min1=0, min2=0, ampm=0, time=0;
uint8_t alarm_hr1=0, alarm_hr2=0, alarm_min1=0, alarm_min2=0;
uint8_t alarm2_hr1=0, alarm2_hr2=0, alarm2_min1=0, alarm2_min2=0;
uint8_t alarm_hr=0, alarm_min=0,alarm2_hr=0, alarm2_min=0;
char info;

void rtc_write(unsigned char addr,unsigned char data)
{
	
	TWBR = 0x0F;                                        //SCL hz value set (7.3 khz)
	TWSR = 0x00;                                        //prescalar value set(4)
	
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTA)|(1<<TWEN);   //Send start condition of I2C
	while(!(TWCR &(1<<TWINT)));                         //wait untill start condition send operation
	
	TWDR = 0xD0;                                        //load the slave addr(RTC) to be written
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	TWDR = addr;                                        //load the RTC_NV_RAM addr
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	TWDR = data;                                        //load the data
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTO)|(1<<TWEN);   //STOP the I2C,yet disabled
	
	//TWCR=0x00; //-- stop and disable I2C              //VERY IMPORTANT , NEED TO CHECK
	
	

}
void rtc_read(unsigned char addr,unsigned char *data)
{
	
	TWBR = 0x0F;                                        //SCL hz value set (7.3 khz)
	TWSR = 0x00;                                        //prescalar value set(4)
	
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTA)|(1<<TWEN);   //Send start condition of I2C
	while(!(TWCR &(1<<TWINT)));                         //wait untill start condition send operation
	
	TWDR = 0xD0;                                        //load the slave addr(RTC)
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	TWDR = addr;                                        //load the RTC_NV_RAM addr
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTA)|(1<<TWEN);   //Send start condition of I2C
	while(!(TWCR &(1<<TWINT)));                         //wait untill start condition send operation
	
	TWDR = 0xD1;                                       //load the slave addr(RTC) to be READ
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	TWCR = (1<<TWINT)|(1<<TWEN);                        //Trigger send operation
	while(!(TWCR &(1<<TWINT)));                         //wait untill send operation to be finish
	
	*data = TWDR;
	
	TWCR = (1<<TWINT)|(1<<TWEA)|(1<<TWSTO)|(1<<TWEN);   //STOP the I2C,yet disabled
	
	//TWCR = 0x00;                                      //VERY IMPORTNAT , NEED TO CHECK AGAIN
	
}
void display_day(unsigned char temp)
{
	unsigned char temp1;
	temp1 = bcd2dec(temp);
	LCD_PutString(min_sec[temp1]);
}
void display_month(unsigned char temp)
{
	unsigned char temp1;
	temp1 = bcd2dec(temp);
	LCD_PutString(min_sec[temp1]);
}
void display_min_sec(unsigned char temp)
{
 	unsigned char temp1;
	temp1 = bcd2dec(temp);
	LCD_PutString(min_sec[temp1]);
}
void display_hour(unsigned char temp)
{
	unsigned char temp1;
	temp&=0x1F;
	temp1=bcd2dec(temp);
	LCD_PutString(hour[temp1]);
}
void display_ampm(unsigned char temp)
{
	unsigned char temp1;
	temp = (temp>>5) & 0x01;
	temp1 = bcd2dec(temp);
	LCD_PutString(am_pm[temp1]);
}

// Convert Decimal to Binary Coded Decimal (BCD)
char dec2bcd(char num)
{
	return ((num/10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
char bcd2dec(char num)
{
	return ((num/16 * 10) + (num % 16));
}
#endif /* RTC_H_ */