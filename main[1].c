/*
 * main.c
 *
 * Created: 5/10/2023 7:25:10 PM
 *  Author: Gagan
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

int ShaftCountRight;

int ShaftCountLeft;

void motion_pin_config (void)
{
	DDRA = DDRA | 0x0F; //set direction of the PORTA 3 to PORTA 0 pins as output
	PORTA = PORTA & 0xF0; // set initial value of the PORTA 3 to PORTA 0 pins to logic 0
	DDRL = DDRL | 0x18; //Setting PL3 and PL4 pins as output for PWM generation
	PORTL = PORTL | 0x18; //PL3 and PL4 pins are for velocity control using PWM
}

void timer1_init(void){
	TCCR1B|= (1<<WGM12) |(1<<CS12) |(1<<CS10);
	OCR1A=15625;
	TIMSK1|=(1<<OCIE1A);
}

/*ISR(TIMER1_COMPA_vect){
	count++;
}*/
void left_encoder_pin_config (void)
{
	DDRE = DDRE & 0xEF; //Set the direction of the PORTE 4 pin as input
	PORTE = PORTE | 0x10; //Enable internal pull-up for PORTE 4 pin
}
void right_encoder_pin_config (void)
{
	DDRE = DDRE & 0xDF; //Set the direction of the PORTE 5 pin as input
	PORTE = PORTE | 0x20; //Enable internal pull-up for PORTE 5 pin
}

void left_position_encoder_interrupt_init (void) //Interrupt 4 enable
{
	cli(); //Clears the global interrupt
	EICRB = EICRB | 0x02; // INT4 is set to trigger with falling edge
	EIMSK = EIMSK | 0x10; // Enable Interrupt INT4 for left position encoder
	sei(); // Enables the global interrupt
}
void right_position_encoder_interrupt_init (void) //Interrupt 5 enable
{
	cli(); //Clears the global interrupt
	EICRB = EICRB | 0x08; // INT5 is set to trigger with falling edge
	EIMSK = EIMSK | 0x20; // Enable Interrupt INT5 for right position encoder
	sei(); // Enables the global interrupt
}

void init_devices()
{
	cli(); //Clears the global interrupt
	motion_pin_config();
	timer1_init();
	left_position_encoder_interrupt_init();
	right_position_encoder_interrupt_init();
	sei(); // Enables the global interrupt
}


//have to add isr

ISR(INT5_vect)
{
	ShaftCountRight++; //increment right shaft position count
}

ISR(INT4_vect)
{
	ShaftCountLeft++; //increment left shaft position count
}

//isr to be added above


void motion_set (unsigned char Direction)
{
	unsigned char PortARestore = 0;
	Direction &= 0x0F; // removing upper nibbel as it is not needed
	PortARestore = PORTA; // reading the PORTA's original status
	PortARestore &= 0xF0; // setting lower direction nibbel to 0
	PortARestore |= Direction; // adding lower nibbel for direction command and
	// restoring the PORTA status
	PORTA = PortARestore; // setting the command to the port
}
void forward (void) //both wheels forward
{
	motion_set(0x06);
}
void back (void) //both wheels backward
{
	motion_set(0x09);
}
void left (void) //Left wheel backward, Right wheel forward
{
	motion_set(0x05);
}
void right (void) //Left wheel forward, Right wheel backward
{
	motion_set(0x0A);
}
void soft_left (void) //Left wheel stationary, Right wheel forward
{
	motion_set(0x04);
}
void soft_right (void) //Left wheel forward, Right wheel is stationary
{
	motion_set(0x02);
}
void soft_left_2 (void) //Left wheel backward, right wheel stationary
{
	motion_set(0x01);
}
void soft_right_2 (void) //Left wheel stationary, Right wheel backward
{
	motion_set(0x08);
}
void stop (void) //hard stop if PORTL 3 and PORTL 4 pins are at logic 1
{
	motion_set(0x00);
}

//function to give a linear motion to the bot(forward or backward)
void linear_distance_mm(unsigned int DistanceInMM)
{
	float ReqdShaftCount = 0;
	unsigned long int ReqdShaftCountInt = 0;
	ReqdShaftCount = DistanceInMM / 5.338; // division by resolution to get shaft count
	ReqdShaftCountInt = (unsigned long int) ReqdShaftCount;
	
	ShaftCountRight = 0;
	while(1)
	{
		if(ShaftCountRight > ReqdShaftCountInt)
		{
			break;
		}
	}
	stop(); //Stop robot
}

//to rotate bot by any angele required
void angle_rotate(unsigned int Degrees)
{
	float ReqdShaftCount = 0;
	unsigned long int ReqdShaftCountInt = 0;
	ReqdShaftCount = (float) Degrees/ 4.090; // division by resolution to get shaft count
	ReqdShaftCountInt = (unsigned int) ReqdShaftCount;
	ShaftCountRight = 0;
	ShaftCountLeft = 0;
	while (1)
	{
		if((ShaftCountRight >= ReqdShaftCountInt) | (ShaftCountLeft >= ReqdShaftCountInt))
		break;
	}
	stop(); //Stop robot
}

//to go forward
void forward_mm(unsigned int DistanceInMM)
{
	forward();
	linear_distance_mm(DistanceInMM);
}

//to go backward
void back_mm(unsigned int DistanceInMM)
{
	back();
	linear_distance_mm(DistanceInMM);
}



void soft_left_degrees(unsigned int Degrees)
{
	// 176 pulses for 360 degrees rotation 2.045 degrees per count
	soft_left(); //Turn soft left
	Degrees=Degrees*2;
	angle_rotate(Degrees);
}

int main(){
	init_devices();

	int distanceTravelled = 0;

	while (distanceTravelled <= 180)
	{
		forward_mm(300);
		distanceTravelled+=30;
		soft_left_degrees(60);
	}
	stop();

}

