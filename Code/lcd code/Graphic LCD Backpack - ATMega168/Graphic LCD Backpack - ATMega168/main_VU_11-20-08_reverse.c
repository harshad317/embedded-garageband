/*
    graphic LCD backpack test code
	
	set up for the big LCD, 160x128
	
	10/16/08, everything works. Pixel, line, circle, set x, set y, print_char, clear_screen. All good.
	
	11/12/08 code jacked for VU meter
	
	11/18/08 reverse display
	
	11/20/08 testing backlight cycling
*/


#include <avr/io.h>
#include "rprintf.h"
#include <math.h>
#include <avr/interrupt.h>

#define FOSC 16000000// Clock Speed
#define BAUD 57600
#define MYUBRR FOSC/16/BAUD-1

#define WR 0	//PC0
#define RD 1	//PC1
#define CE 2	//PC2
#define CD 3	//PC3
#define HALT 4	//PC4
#define RST 5	//PC5

#define PEAK 1

#define BL_EN 2	//PB2

#define X_ENDPOINT 159
#define Y_ENDPOINT 127

//#define	CS20 0
//#define	CS21 1
//#define	CS22 2


//Define functions
//======================
void ioinit(void);      //Initializes IO
void delay_ms(uint16_t x); //General purpose delay
void delay(void);
void delay_us(uint8_t x);
void USART_Init( unsigned int ubrr);
void put_char(char byte);
void print_num(short num);
int rnd(float number);

void set_data(char data);
char read(char D_S);//reads data (D_S = 1) or status (D_S = anything else)
void write(char D_C, char byte);//writes data or command
void display_init(void);


void clear_screen(void);

void print_char(char S_R, char txt);
void del_char(char endpoint);
void pixel(char S_R, char x, char y);
void line(char S_R, char x1, char y1, char x2, char y2);
void circle(char S_R, int x, int y, int r);
void demo(void);

void erase_block(char x1, char y1, char x2, char y2);
void box(char x1, char y1, char x2, char y2);

int get_adc(void);

//======================
char x_offset = 0;
char y_offset = 127;
char reverse = 1;

unsigned char RX_array[256];
unsigned char BL_dutycycle = 100;
volatile unsigned short RX_in = 0;
unsigned short RX_read = 0;

static char logo[30] = {0x01,0xC0,0x03,0x80,0x03,0x80,0x01,0xD0,
						0x01,0xF8,0x0C,0xF8,0x18,0xF8,0x1F,0xF8,
						0x1F,0xF8,0x1F,0xF0,0x1F,0xE0,0x1F,0xC0,
						0x1C,0x00,0x18,0x00,0x10,0x00};

//Jacked from Sinister 7 code
static char text_array[130] = {0x00,0x00,0x00,0x00,0x00,/*space*/
                              0x00,0xF6,0xF6,0x00,0x00,/*!*/
                              0x00,0xE0,0x00,0xE0,0x00,/*"*/
                              0x28,0xFE,0x28,0xFE,0x28,/*#*/
                              0x00,0x64,0xD6,0x54,0x08,/*$*/
                              0xC2,0xCC,0x10,0x26,0xC6,/*%*/
                              0x4C,0xB2,0x92,0x6C,0x0A,/*&*/
                              0x00,0x00,0xE0,0x00,0x00,/*'*/
                              0x00,0x38,0x44,0x82,0x00,/*(*/
                              0x00,0x82,0x44,0x38,0x00,/*)*/
                              0x88,0x50,0xF8,0x50,0x88,/***/
                              0x08,0x08,0x3E,0x08,0x08,/*+*/
                              0x00,0x00,0x05,0x06,0x00,/*,*/
                              0x08,0x08,0x08,0x08,0x08,/*-*/
                              0x00,0x00,0x06,0x06,0x00,/*.*/
                              0x02,0x0C,0x10,0x60,0x80,/*/*/
                              0x7C,0x8A,0x92,0xA2,0x7C,/*0*/
                              0x00,0x42,0xFE,0x02,0x00,/*1*/
                              0x42,0x86,0x8A,0x92,0x62,/*2*/
                              0x44,0x82,0x92,0x92,0x6C,/*3*/
                              0x10,0x30,0x50,0xFE,0x10,/*4*/
                              0xE4,0xA2,0xA2,0xA2,0x9C,/*5*/
                              0x3C,0x52,0x92,0x92,0x0C,/*6*/
                              0x80,0x86,0x98,0xE0,0x80,/*7*/
                              0x6C,0x92,0x92,0x92,0x6C,/*8*/
                              0x60,0x92,0x92,0x94,0x78};/*9*/
							  
							  
static char line_array[284] = {10,80,40,37,
								12,81,41,38,
								14,82,42,38,
								16,83,43,39,
								18,84,45,40,
								20,85,46,40,
								22,86,47,41,
								24,87,48,41,
								26,88,49,42,
								28,89,50,42,
								30,90,51,42,
								32,91,53,43,
								34,92,54,44,
								36,92,55,44,
								38,93,56,44,
								40,94,57,45,
								42,94,58,45,
								44,95,59,45,
								46,95,61,46,
								48,96,62,46,
								50,96,63,46,
								52,97,64,46,
								54,97,65,47,
								56,98,66,47,
								58,98,67,47,
								60,98,69,47,
								62,99,70,47,
								64,99,71,47,
								66,99,72,48,
								68,99,73,48,
								70,100,74,48,
								72,100,75,48,
								74,100,77,48,
								76,100,78,48,
								78,100,79,48,
								80,100,80,48,
								82,100,81,48,
								84,100,82,48,
								86,100,83,48,
								88,100,85,48,
								90,100,86,48,
								92,99,87,48,
								94,99,88,48,
								96,99,89,47,
								98,99,90,47,
								100,98,91,47,
								102,98,93,47,
								104,98,94,47,
								106,97,95,47,
								108,97,96,46,
								110,96,97,46,
								112,96,98,46,
								114,95,99,46,
								116,95,101,45,
								118,94,102,45,
								120,94,103,45,
								122,93,104,44,
								124,92,105,44,
								126,92,106,44,
								128,91,107,43,
								130,90,109,42,
								132,89,110,42,
								134,88,111,42,
								136,87,112,41,
								138,86,113,41,
								140,85,114,40,
								142,84,115,40,
								144,83,117,39,
								146,82,118,38,
								148,81,119,38,
								150,80,120,37};









ISR (SIG_USART_RECV)									//USART Receive Interrupt
{
	cli();//Disable Interrupts
	RX_array[RX_in] = UDR0;
	
	RX_in++;
	
	if (RX_in >= 256) RX_in = 0;
	
	sei();//Enable Interrupts
	
}

ISR (TIMER0_COMPA_vect)
{
	//unsigned char y;
	
	cli();//Disable Interrupts
	
	TIFR0 = (1<<OCF0A);//clear the interrupt
	
	PORTB &= (~(1<<BL_EN));//on
	
	//y = PINB;
	//if (y & (1<<BL_EN)) PORTB &= (~(1<<BL_EN));
	//else PORTB |= (1<<BL_EN);
	//PORTB |= (1<<BL_EN);
	
	TIMSK0 = (1<<OCIE0B);//disable A, enable B
	sei();//Enable Interrupts
}


ISR (TIMER0_COMPB_vect)
{
	//unsigned char y;
	
	cli();//Disable Interrupts
	
	TIFR0 = (1<<OCF0B);//clear the interrupt
	
	//y = PINB;
	//if (y & (1<<BL_EN)) PORTB &= (~(1<<BL_EN));
	//else PORTB |= (1<<BL_EN);
	//PORTB |= (1<<BL_EN);
	
	PORTB |= (1<<BL_EN);//off
	
	TIMSK0 = (1<<OCIE0A);//disable B, enable A
	sei();//Enable Interrupts
}





int main (void)
{
	char x, y, a, temp;
	//short a, b;
	float temp_y, temp_y2, temp_x;
	int q;
	
	//char a = 0;
    ioinit(); //Setup IO pins and defaults
	//USART_Init( MYUBRR);
	//rprintf_devopen(put_char); /* init rrprintf */
	
	//reset the display
	delay_ms(1);
	PORTC |= (1<<RST);
	
	//initialize the display
	display_init();

	clear_screen();
	
	//Backlight on
	PORTB &= (~(1<<BL_EN));
	
	//circle(1,80,-20,142);
	//circle(1,80,-20,70);
	
	//top arcs=======================================================================
	for (x = 0; x < 160; x++)
	{
		//temp_y = ((sqrt((radius^2) - ((x - (x offset))*(x - (x offset))))) - (y offset))
		//...x ofset and y offset are NOT the test offsets. This is just here as an example.
		temp_y = ((sqrt((23716) - ((x - 80)*(x - 80)))) - 30);
		temp_y2 = ((sqrt((23409) - ((x - 80)*(x - 80)))) - 30);
		
		//temp_y *= (-1);
		pixel(1, x, (char)(temp_y));
		pixel(1, x, (char)(temp_y2));
	}
	
	for (x = 0; x < 160; x++)
	{
		//temp_y = ((sqrt((r^2) - ((x - 80)*(x - 80)))) - 20)
		temp_y = ((sqrt((20164) - ((x - 80)*(x - 80)))) - 30);
		temp_y2 = ((sqrt((19881) - ((x - 80)*(x - 80)))) - 30);
		
		//temp_y *= (-1);
		pixel(1, x, (char)(temp_y));
		pixel(1, x, (char)(temp_y2));
	}
	
	
	//bottom arc=======================================================================
	for (x = 40; x < 120; x++)
	{
		temp_y = ((sqrt((4900) - ((x - 80)*(x - 80)))) - 30);
		temp_y2 = ((sqrt((4761) - ((x - 80)*(x - 80)))) - 30);
		
		//temp_y *= (-1);
		pixel(1, x, (char)(temp_y));
		pixel(1, x, (char)(temp_y2));
	}
	
	
	//peak arc=======================================================================
	for (x = 108; x < 156; x++)
	{
		//temp_y = ((sqrt((19044) - ((x - 80)*(x - 80)))) - 30);
		temp_y2 = ((sqrt((18225) - ((x - 80)*(x - 80)))) - 30);
		
		//temp_y *= (-1);
		//pixel(1, x, (char)(temp_y));
		pixel(1, x, (char)(temp_y2));
	}
	
	//lines on the meter sides=======================================================================
	line(1,0,87,40,27);
	line(1,1,87,41,27);
	line(1,120,27,159,87);
	line(1,119,27,158,87);
	
	
	//little cleanup================================================================
	pixel(1,80,122);
	pixel(0,80,124);
	
	pixel(1,80,110);
	pixel(0,80,112);
	
	pixel(1,80,38);
	pixel(0,80,40);
	
	
	//numbers================================================================
	x_offset = 1, y_offset = 99;
	print_char(1,'2');
	y_offset = 101;
	print_char(1,'0');
	
	line(1, 9, 92, 12, 86);
	line(1, 8, 92, 11, 86);
	
	x_offset = 24, y_offset = 111;
	print_char(1,'1');
	y_offset = 112;
	print_char(1,'0');
	
	line(1,30, 101, 32, 95);
	line(1, 31, 101, 33, 95);
	
	x_offset = 48, y_offset = 117;
	print_char(1,'7');
	
	line(1,50, 108, 51, 101);
	line(1, 51, 108, 52, 101);

	x_offset = 66, y_offset = 119;
	print_char(1,'5');
	
	line(1,68, 109, 69, 103);
	line(1, 69, 109, 70, 103);
	
	x_offset = 84, y_offset = 119;
	print_char(1,'3');
	
	line(1,86, 109, 86, 103);
	line(1, 87, 109, 87, 103);
	
	x_offset = 110, y_offset = 116;
	print_char(1,'0');
	
	line(1,108, 102, 109, 106);
	line(1,109, 102, 110, 106);
	
	x_offset = 135, y_offset = 108;
	print_char(1,'3');
	
	line(1,132, 93, 135, 100);
	line(1,133, 93, 136, 100);
	
	
	//logo==================================================================
	y = 28;
		
	q = 0;
	while(q < 30)
	{
		temp = logo[q];
		for (x = 72; x < 80; x++)
		{
			if (temp & 0x80) pixel(1,x,y);
			
			temp <<= 1;
		}
		q++;
		temp = logo[q];
		for (x = 80; x < 88; x++)
		{
			if (temp & 0x80) pixel(1,x,y);
			
			temp <<= 1;
		}
		y--;
		q++;
	}	
	
	x = 0;
	line(1, line_array[x*4], line_array[x*4+1], line_array[x*4+2], line_array[x*4+3]);
	
	while(1)
	{
		q = get_adc();
		
		y = (char)(rnd(q / 13.5)-10);
		
		delay_ms(10);
		
		if (y != x)
		{
			line(0, line_array[x*4], line_array[x*4+1], line_array[x*4+2], line_array[x*4+3]);
			line(1, line_array[y*4], line_array[y*4+1], line_array[y*4+2], line_array[y*4+3]);
			x = y;
			
			if (y >= 50) PORTD |= (1<<PEAK);
			else PORTD &= (~(1<<PEAK));
		}
		
		
	}
	
	while(1);

}

void ioinit (void)
{
	
    //1 = output, 0 = input
	
	/*
	WR	//PC0
	RD	//PC1
	CE	//PC2
	C_D //PC3
	HALT	//PC4
	RST	//PC5
	*/

	PORTB |= (1<<BL_EN);//Backlight off
	DDRB |= (1<<BL_EN);//set PB2 as output
	
	//set these in the read/write functions instead of here
	//DDRB = 0b00000011; //PB0 and PB1 are outs
	//DDRD = 0b11111100; //PD2-PD7 are also outs.  Ports B and D are the data bus.

	PORTC = ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT));
	PORTC &= (~(1<<RST));//set the reset line low at power up
	DDRC = ((1<<WR) | (1<<RD) | (1<<CE) | (1<<CD) | (1<<HALT) | (1<<RST));
	
	DDRD = (1<<PEAK);
	PORTD &= (~(1<<PEAK));
	
	//Init timer 2
    //8,000,000 / 8 = 1,000,000
    TCCR2B = (1<<CS21); //Set Prescaler to 8. CS21=1
	//TCCR2 = ((1<<CS20) | (1<<CS22) | (1<<CS22));
	
	//Set up Timer 0
	TCCR0A = (1<<WGM01);//CTC mode
	TCCR0B = (1<<CS01);
	TIMSK0 = (1<<OCIE0A);
	OCR0B = BL_dutycycle;
	
	OCR0A = 100 - BL_dutycycle;
	
	SREG |= 0x80;
}

//General short delays
void delay_ms(uint16_t x)
{
	for (; x > 0 ; x--)
    {
        delay_us(250);
        delay_us(250);
        delay_us(250);
        delay_us(250);
    }
	
}

//General short delays
void delay_us(uint8_t x)
{
	char temp;
	
	if (x == 0) temp = 1;
	else temp = x;
    //TIFR = 0x01; //Clear any interrupt flags on Timer2
	TIFR2 |= 0x01;
    
    TCNT2 = 256 - temp; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click

    //while( (TIFR & (1<<TOV2)) == 0);
	while(!(TIFR2 & 0x01));
	
	if (x == 0) return;//this is for display timing
	
	
	//The prescaler doesn't allow for a setting of 16, just 8 or 32. So, we do this twice.
	TIFR2 |= 0x01;
    
    TCNT2 = 256 - temp; //256 - 125 = 131 : Preload timer 2 for x clicks. Should be 1us per click

    //while( (TIFR & (1<<TOV2)) == 0);
	while(!(TIFR2 & 0x01));
	
}


void USART_Init( unsigned int ubrr)
{
	// Set baud rate 
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	// Enable receiver and transmitter 
	//UCSRB = (1<<RXEN)|(1<<TXEN);
	UCSR0B = (1<<RXCIE0)|(1<<RXEN0)|(1<<TXEN0);	//Enable Interrupts on receive character
	// Set frame format: 8data, 2stop bit 
	//UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	UCSR0C = (1<<UMSEL00)|(1<<UCSZ00)|(1<<UCSZ01);
	sei();
}



void put_char(char byte)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) );
	/* Put data into buffer, sends the data */
	UDR0 = byte;
}

//delay for display timing
void delay(void)
{
	char y;
	
	for(y = 0; y < 20; y++)
	{
		asm volatile ("nop");
		
	}
	
	/*
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("nop");
	*/
	
}

//set data port
void set_data(char data)
{
	//PORTB
	//DB0 = PB0
	//DB1 = PB1
	
	PORTB &= 0xFC;
	
	//PORTD
	//DB2 = PD2
	//DB3 = PD3
	//DB4 = PD4
	//DB5 = PD5
	//DB6 = PD6
	//DB7 = PD7
	
	PORTD &= 0x03;
	
	PORTB |= (data & 0x03);
	PORTD |= (data & 0xFC);

}


//Reads data or status
//for data D_S = 1, for status D_S = 0
//returns the value of the data bus
char read(char D_S)
{
	char data1 = 0, data2 = 0;
	
	DDRB &= 0xFC;//PB0 and PB1 inputs
	DDRD &= 0x02;//everything but PD1 as input
	
	PORTC &= ~((1 << RD) | (1 << CE));//CD high for status
	if (D_S == 1) PORTC &= ~(1 << CD);//CD down for data
	
	delay_us(0);
	
	data1 = PINB;
	data1 &= 0x03;
	
	data2 = PIND;
	data2 &= 0xFC;
	
	data1 |= data2;
	
	PORTC |= ((1 << CD) | (1 << RD) | (1 << CE));//all up
	
	delay_us(0);
	
	return data1;

}


//Writes data (D_C = 1) or command (D_C = anything else)
void write(char D_C, char byte)
{
	DDRB |= 0x03; //PB0 and PB1 are outs
	DDRD |= 0xFC; //PD2-PD7 are also outs.  Ports B and D are the data bus
	
	set_data(byte);
	
	if (D_C == 1) PORTC &= ~((1 << WR) | (1 << CE) | (1 << CD));//down
	else PORTC &= ~((1 << WR) | (1 << CE));//down
	
	delay_us(0);
	PORTC |= ((1 << CD) | (1 << WR) | (1 << CE));//all up
	delay();
	DDRB &= 0xFC;//PB0 and PB1 inputs
	DDRD &= 0x02;//everything but PD1 as input
	
	delay_us(0);

}



void display_init(void)
{
	//set graphics home address to 0
	while(!(read(0) & 3));//read status
	write(1, 0);
	while(!(read(0) & 3));//read status
	write(1, 0);
	while(!(read(0) & 3));//read status
	write(0, 0x42);

	//set graphics area
	while(!(read(0) & 3));//read status
	write(1, 20);//20 bytes, 160/8
	while(!(read(0) & 3));//read status
	write(1, 0);
	while(!(read(0) & 3));//read status
	write(0, 0x43);
	
	//set mode
	while(!(read(0) & 3));//read status
	write(0, 0x80);//Or, with internal character generator
	
	//set display mode
	while(!(read(0) & 3));//read status
	write(0, 0x98);//Graphics on

}





void clear_screen(void)
{
	int x;
	
	//set address pointer to 0, start of graphics
	while(!(read(0) & 3));//read status
	write(1, 0);
	while(!(read(0) & 3));//read status
	write(1, 0);
	while(!(read(0) & 3));//read status
	write(0, 0x24);
	
	for(x = 0; x < 0xA00; x++)
	{
		while(!(read(0) & 3));//read status
		if (reverse == 1) write(1,0xFF);
		else if (reverse == 0) write(1, 0);		
		while(!(read(0) & 3));//read status
		write(0, 0xC0);
	}
	
	x_offset = 0;
	y_offset = 127;
}

/*
void set_cursor(char x_spot, char y_spot)
{
	//set address pointer to 0, start of graphics
	while(!(read(0) & 3));//read status
	write(1, x_spot);
	while(!(read(0) & 3));//read status
	write(1, y_spot);
	while(!(read(0) & 3));//read status
	write(0, 0x21);

}
*/

void pixel(char S_R, char x, char y)
{
	short address = 0;
	char byte = 0;
	
	if (reverse == 1) S_R ^= 1;
		
	//don't try to print something outside of our range
	if ((x > 159) | (y > 127)) return;
	
	address = ((127-y) * 20) + (x / 8);
	
	//set address pointer
	while(!(read(0) & 3));//read status
	byte = (char)(address & 0xFF);

	write(1, byte);//20 bytes, 160/8
	
	while(!(read(0) & 3));//read status
	byte = (char)((address & 0xFF00) >> 8);

	write(1, byte);
	
	while(!(read(0) & 3));//read status
	write(0, 0x24);
	
	byte = ~(x % 8);

	byte |= 0xF8;
	if (S_R == 0) byte &= 0xF7;
	
	//set-reset bit
	while(!(read(0) & 3));//read status
	write(0, byte);
	
}



void line(char S_R, char x1, char y1, char x2, char y2)
{
	float m, q;
	int x_dif, y_dif;
	int a, b, c;
	
	//if ((x1 > X_ENDPOINT) | (x2 > X_ENDPOINT)) return;
	//if ((x1 < 0) | (x2 < 0)) return;
	//if ((y1 > Y_ENDPOINT) | (y2 > Y_ENDPOINT)) return;
	//if ((y1 < 0) | (y2 < 0)) return;
	
	x_dif = x2 - x1;
	y_dif = y2 - y1;
	if (y_dif < 0) y_dif *= (-1);
	

	m = (float)(y2 - y1) / (float)(x2 - x1);
	
	b = y1-(m*x1);
	
	if(x_dif >= y_dif)
	{
		for (a = x1; a <= x2; a++)
		{
			pixel(S_R, (char)a, (char)((m*a)+b));
			//delay_ms(25);
		}
	}
	
	else
	{
		if (y2 > y1)
		{
			for (a = y1; a <= y2; a++)
			{
				if (x_dif == 0) c = x1;
				else
				{
					q = (((float)(a-b))/m);
					c = rnd(q);
				}
				
				pixel(S_R, (char)c, (char)a);
				//delay_ms(25);
			}
		}
		
		else if (y1 > y2)
		{
			for (a = y1; a >= y2; a--)
			{
				if (x_dif == 0) c = x1;
				else 
				{
					q = (((float)(a-b))/m);
					c = rnd(q);
				}
			
				pixel(S_R, (char)c, (char)a);
				//delay_ms(25);
			}
		}
	}
		
}



void circle(char S_R, int x, int y, int r)
{
	int x1 = 0, x2 = 0;
	int x_line = 0, y_line = 0;
	int temp_y;
	int temp_x;
	//float x_float, y_float, r_float;
	
	x1 = x - r;
	x2 = x + r;
	
	for (temp_x = x1; temp_x <= x2; temp_x++)
	{
		temp_y = ((sqrt((r*r) - ((temp_x - x)*(temp_x - x)))) - y);
		
		temp_y *= (-1);
		
		/*
		print_num((short)temp_x);
		put_char(9);
		print_num((short)temp_y);
		put_char(10);
		put_char(13);
		*/
		if (temp_x > x1)
		{
			//line(S_R, (char)x_line, (char)y_line, (char)temp_x, (char)temp_y);
			line(S_R, (char)x_line, (char)(2*y - y_line), (char)temp_x, (char)(2*y - temp_y));
		}
			
		else 
		{
			//pixel(S_R, (char)temp_x, (char)temp_y);
			pixel(S_R, (char)temp_x, (char)(y + y - temp_y));
		}
		
		x_line = temp_x;
		y_line = temp_y;
		
		//
		
		
	}
	

}


int rnd(float number)
{
	int a;
	float b;
	
	a = number / 1;
	b = number - a;
	
	if (b >= 0.5) a++;
	
	return a;

}


void print_char(char S_R, char txt)
{
    short text_array_offset = (txt - 32)*5, j;
    char x, k;
	
	
    for (j = text_array_offset; j < text_array_offset+5; j++)
    {
		
		
		k = text_array[j];
		
		for (x = 0; x < 8; x++)
		{
			if(k & 0x80) pixel(S_R, x_offset, y_offset - x);
			k <<= 1;
		}
			
		//if (j == text_array_offset+5) x_offset++;//blank byte for letter spacing

			
		x_offset++;
		
    }
	
	x_offset++;
	
    if ((x_offset + 6) > 159)
	{
		x_offset = 0;
		if (y_offset <= 7) y_offset = 127;
		else y_offset -= 8;
		
	}
	
}



void print_num(short num)
{
	short a;
	char b, c;
	a = num;
	
	//print hex
	for (c = 0; c < 4; c++)
	{
		b = (char)((a & 0xF000) >> 12);
		if (b < 10) put_char(b+48);
		else put_char(b+55);
		a <<= 4;
	}
	
	//decimal
	/*
	b = a/100;
	putchr(b+48);
	a -= b*100;
	b = a/10;
	putchr(b+48);
	a -= b*10;
	putchr(a+48);
	*/
	
}


void demo(void)
{
	char x, y, temp;
	int q = 0;
	
	while(1)
	{	
		x_offset = 0;
		y_offset = 127;
	
		for (y = 0; y < 5; y++)
		{
			for (x = 32; x < 123; x++)
			{	
				del_char(1);
				print_char(1, x);
				if (RX_in > 0) return;
			}
		}
		
		clear_screen();
		
		for (y = 0; y < 5; y++)
		{
			for (x = 32; x < 123; x++)
			{
				//x_offset += 4;
				y_offset -= 6;
				if (y_offset <= 8) y_offset = 127;
				del_char(1);
				print_char(1, x);
				if (RX_in > 0) return;
			}
		}
		
		clear_screen();
		
		//draw circles================================
		for (x = 5; x < 120; x += 5)
		{
			circle(1,80,64,x);
			if (RX_in > 0) return;
		}
		
		
		//draw lines===================================
		y = Y_ENDPOINT;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(1,0,y,x,0);
			y -= 16;
		}
		
		y = 0;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(1,x,0,X_ENDPOINT,y);
			y += 16;
		}
		
		y = Y_ENDPOINT;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(1,x,Y_ENDPOINT,X_ENDPOINT,y);
			y -= 16;
		}
		
		y = 0;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(1,0,y,x,Y_ENDPOINT);
			y += 16;
		}
		
		
		//erase circles================================
		for (x = 5; x < 120; x += 5)
		{
			circle(0,80,64,x);
			if (RX_in > 0) return;
		}

		//erase lines===================================
		y = Y_ENDPOINT;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(0,0,y,x,0);
			y -= 16;
		}
		
		y = 0;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(0,x,0,X_ENDPOINT,y);
			y += 16;
		}
		
		y = Y_ENDPOINT;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(0,x,Y_ENDPOINT,X_ENDPOINT,y);
			y -= 16;
		}
		
		y = 0;
		
		for (x = 0; x < X_ENDPOINT; x += 20)
		{
			line(0,0,y,x,Y_ENDPOINT);
			y += 16;
		}
		
		if (RX_in > 0) return;
		
		//Boxes=================================================================
		y = 111;
		for (x = 0; x <= 140; x += 10)
		{
			erase_block(x, y, x+16, y+16);
			box(x, y, x+16, y+16);
			y -= 7;
		}
		
		
		//x = 110;
		y = 28;
		//Logo=================================================================
		q = 0;
		while(q < 30)
		{
			temp = logo[q];
			for (x = 140; x < 148; x++)
			{
				if (temp & 0x80) pixel(1,x,y);
				
				temp <<= 1;
			}
			q++;
			temp = logo[q];
			for (x = 148; x < 156; x++)
			{
				if (temp & 0x80) pixel(1,x,y);
				
				temp <<= 1;
			}
			y--;
			q++;
		}	
		
		delay_ms(3000);
		clear_screen();
		
	}
	

}

//Deletes a character. Endpoint == 0 for a backwards delete,
//Endpoint != 0 to erase spot for a new character write
void del_char(char endpoint)
{
	char a, y;
	
	if (endpoint == 0)//Backwards delete
	{
		if (x_offset <= 5)
		{
			//x_offset = 150;
			//if (y_offset >= 120) y_offset = 7;
			//else y_offset += 8;
			
			x_offset += 152;
			y_offset += 8;
			
			if (y_offset > 127) y_offset -= 128;
		}
		
		else x_offset -= 6;
	}
	/*	
	for (a = 0; a < 6; a++)
	{					
		for (y = 0; y < 8; y++)
		{
			pixel(0, x_offset + a, y_offset - y);
			
		}
	}
	*/
	
	for (a = x_offset; a < x_offset + 6; a++)
	{					
		for (y = y_offset - 7; y <= y_offset; y++)
		{
			pixel(0, a, y);
			//put_char('L');
		}
	}
		
}


void erase_block(char x1, char y1, char x2, char y2)
{
	static char temp_x = 0, temp_y = 0;
	
	for (temp_y = y2; temp_y >= y1; temp_y--)
	{
		for (temp_x = x1; temp_x <= x2; temp_x++)
		{
			pixel(0, temp_x, temp_y);
			
			//rprintf("%d ",temp_x);
			//rprintf("%d \r\n",temp_y);
			//delay_ms(1000);
			
		}
	}	
	
	

}

void box(char x1, char y1, char x2, char y2)
{
	//static char temp_x = 0, temp_y = 0;
	
	line(1, x1, y1, x1, y2);
	line(1, x1, y1, x2, y1);
	line(1, x2, y1, x2, y2);
	line(1, x1, y2, x2, y2);

}


int get_adc(void)
{
	//ADC result vars
	int l;//x low register
	//int h;//x high register
	int l2;//temp low
	int h2;//temp high
	
	//int Vref = 502;//reference V in mV

	//adc conversion
	ADMUX = ((1 << MUX1)|(1 << MUX2) | (0x40));//ADC6, AVcc as Vref
	ADCSRA = (1 << ADEN)|(1 << ADSC)|(1<<ADPS2)|(1<<ADPS1);
	
	while(ADCSRA & (1 << ADSC));
	l2 = (ADCL & 0xFF);
	h2 = (ADCH & 0x03);
	//rprintf("%d %d\r\n", h2, l2);
	
	//h2 = h2 << 8;
	//h2 = ((h2 + l2)*Vref)/1024;
	h2 <<= 8;
	h2 |= l2;
	//l = (ADCH & 0x03) << 8;
	//l |= ADCL;
	
	return (int)h2;
	//return 512;
	
	//l = ADCL;
	//h = ADCH & 0x03;
	//h = h << 8;
	//h = ((h + l)*Vref)/512;


}





