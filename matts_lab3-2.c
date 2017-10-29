/* Modified sample code for Lab 3-1, with additional functions from sample code for Lab 3-2.*/
#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>		//Must be included last in header file block
//-----------------------------------------------------------------------------
// 8051 Initialization Functions
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init();
void SMB_Init(void);
void PCA_ISR (void) __interrupt 9;
unsigned int Read_Compass(void);
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
unsigned char h_count;			//Keeps track of how many PCA interrupts occurred (resets at value of 2 or greater)
unsigned char read_counter;		//Keeps track of how many compass reads have occurred
unsigned char new_heading;		//Flag used to keep 40 ms between compass readings
unsigned int heading;			//Stores the value of 0 to 3599 returned by the electronic compass
//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
	//initialize board
	Sys_Init();
	putchar(' '); //the quotes in this line may not format correctly
	Port_Init();
	XBR0_Init();
	PCA_Init();
	SMB_Init();
	
	//print beginning message
	printf("\r\nElectronic compass: reading the direction\r\n");
	
	//Initialize global variables
	h_count = 0;
	new_heading = 0;
	read_counter = 0;
	heading = 0;
	
	while(1)
	{
		if (new_heading) 				//Enough overflows for a new heading
		{
			heading = Read_Compass();
			new_heading = 0;
			if (read_counter >= 5)		//Only prints heading every fifth compass read
			{
				read_counter = 0;
				printf("Compass heading is:%d\r\n", heading);
			}
		}
	}
}
//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
	P1MDOUT |= 0x01;		//set output pin for CEX0 in push-pull mode (P1.0)
}
//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
	XBR0 = 0x27;	//configure crossbar with UART, SPI, SMBus, and CEX channels as
					//in worksheet
}
//-----------------------------------------------------------------------------
// SMB_Init
//-----------------------------------------------------------------------------
//
// Set up the I2C Bus
//
void SMB_Init()
{
	SMB0CR = 0x99;	//Sets SCL to 100 kHz (closer than 0x93 to actual 100 kHz)
	ENSMB = 1;		//Enables SMB
}
//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
	PCA0MD = 0x81;		//Enable CF Interrupt, uses SYSCLK/12, 
	PCA0CPM0 = 0xC2;	//CCM0 in 16-bit compare mode,enables PWM
	PCA0CN |= 0x40;		//Enables PCA counter
	EIE1 |= 0x08;		//Enable PCA interrupt
	EA = 1;				//Enable global interrupt	
}
//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR(void) __interrupt 9
{
	if (CF)
	{
		CF = 0;					//Reset CF interrupt flag
		PCA0 = 28672;			//Starting value for a 20 ms pulse when using SYSCLK/12 and 16-bit timer
		h_count++;
		if(h_count >= 2)
		{
			new_heading = 1;	// 2 overflows is about 40 ms
			h_count = 0;
		}
	}
	
	PCA0CN &= 0x40;		//Handle other PCA interrupt sources
}
//-----------------------------------------------------------------------------
// Read_Compass
//-----------------------------------------------------------------------------
// 
// Takes reading from electronic compass and returns a value between 0 and 3599
//
unsigned int Read_Compass()
{
	unsigned char addr = 0xC0; 			//The address of the sensor, 0xC0 for the compass
	unsigned char Data[2]; 				//Data is an array with a length of 2
	i2c_read_data(addr, 2, Data, 2);	//Read two byte, starting at reg 2
	heading =(((unsigned int)Data[0] << 8) | Data[1]); //combine the two values
	//heading has units of 1/10 of a degree
	read_counter++;		//Increments read_counter used for printing reads
	return heading; 	//The heading returned in degrees between 0 and 3599
}