/*
 * main.c
 *
 * Created: 5/20/2023 3:56:31 PM
 *  Author: dell
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <math.h>

void port_init();
void timer5_init();
void velocity(unsigned char, unsigned char);

unsigned char ADC_Conversion(unsigned char);
unsigned char ADC_Value;
unsigned char flag = 0;
unsigned char LW = 0;
unsigned char CW = 0;
unsigned char RW = 0;

void adc_pin_config (void)
{
	DDRF = 0x00;
	PORTF = 0x00;
	DDRK = 0x00;
	PORTK = 0x00;
}

void motion_pin_config (void)
{
	DDRA = DDRA | 0x0F;
	PORTA = PORTA & 0xF0;
	DDRL = DDRL | 0x18;   
	PORTL = PORTL | 0x18; 
}

void port_init()
{
	adc_pin_config();
	motion_pin_config();
}

void timer5_init()
{
	TCCR5B = 0x00;	
	TCNT5H = 0xFF;	
	TCNT5L = 0x01;	
	OCR5AH = 0x00;	
	OCR5AL = 0xFF;	
	OCR5BH = 0x00;	
	OCR5BL = 0xFF;	
	OCR5CH = 0x00;	
	OCR5CL = 0xFF;	
	TCCR5A = 0xA9;	
	TCCR5B = 0x0B;	
}

void adc_init()
{
	ADCSRA = 0x00;
	ADCSRB = 0x00;		
	ADMUX = 0x20;		
	ACSR = 0x80;
	ADCSRA = 0x86;		
}

unsigned char ADC_Conversion(unsigned char Ch)
{
	unsigned char a;
	if(Ch>7)
	{
		ADCSRB = 0x08;
	}
	Ch = Ch & 0x07;
	ADMUX= 0x20| Ch;
	ADCSRA = ADCSRA | 0x40;		
	while((ADCSRA&0x10)==0);	
	a=ADCH;
	ADCSRA = ADCSRA|0x10; 
	ADCSRB = 0x00;
	return a;
}

void velocity (unsigned char left_motor, unsigned char right_motor)
{
	OCR5AL = (unsigned char)left_motor;
	OCR5BL = (unsigned char)right_motor;
}

void motion_set (unsigned char Direction)
{
	unsigned char PortARestore = 0;

	Direction &= 0x0F; 		
	PortARestore = PORTA; 		
	PortARestore &= 0xF0; 		
	PortARestore |= Direction; 
	PORTA = PortARestore; 		
}

void forward (void)
{
	motion_set (0x06);
}

void stop (void)
{
	motion_set (0x00);
}

void back (void) 
{
 motion_set(0x09);
} 

void left (void)
{
	motion_set(0x05);
}

void right (void)
{
	motion_set(0x0A);
}

void init_devices (void)
{
	cli(); //Clears the global interrupts
	port_init();
	adc_init();
	timer5_init();
	sei();   //Enables the global interrupts
}

int main(void)
{
	init_devices();
	
	while(1)
	{


		LW = ADC_Conversion(3); //Getting data of Left WL Sensor
		CW = ADC_Conversion(2); //Getting data of Center WL Sensor
		RW = ADC_Conversion(1); //Getting data of Right WL Sensor


		
		if (LW > 0x28 && CW <=0x28 && RW > 0x28)//Straight path
		{
			forward();
			velocity(50,50);
		}


		if (LW <= 0x28 && CW >0x28 && RW > 0x28)//Left turn
		{
			velocity(20,40);
		}


		if (LW > 0x28 && CW > 0x28 && RW <= 0x28)//Right Turn
		{
			velocity(40,20);
		}


		if (LW <= 0x28 && CW >0x28 && RW <= 0x28)//T Intersection
		{
			velocity(20,40);// As left is possible
		}


		if (LW <= 0x28 && CW <=0x28 && RW > 0x28)//Left T Intersection
		{
			velocity(20,40);// As Left is possible
		}


		if (LW > 0x28 && CW <=0x28 && RW <= 0x28)//Right T Tntersection
		{
			forward();
			velocity(50,50);//As Straight path is possible
		}


		if (LW > 0x28 && CW >0x28 && RW > 0x28)//Dead End
		{
			back();
			velocity(50,50); //As no other direction is possible
		}


		if (LW <= 0x28 && CW <=0x28 && RW <= 0x28)
		{
			forward();
			velocity(50,50);
			_delay_ms(1000);
			stop();


			if (LW <= 0x28 && CW <=0x28 && RW <= 0x28)
			{   //mazde has ended
				stop();
			}
			else
			{
				velocity(20,40);
			}
		}
	}
}
