/* Sample code for speed control using PWM. */
#include <stdio.h>
#include <c8051_SDCC.h>
#include <i2c.h>

//-----------------------------------------------------------------------------
// 8051 Initialization Functions
//-----------------------------------------------------------------------------
void PCA_Init (void);
void XBR0_Init(void);
void Interrupt_Init(void);
unsigned int ReadRanger();
void PingRanger();
void SMB_Init();

unsigned char addr=0xE0; // the address of the ranger is 0xE0
unsigned char Data[2];
unsigned char r_count=0;
unsigned char light;
//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
	// initialize board
	Sys_Init();
	putchar(' '); //the quotes in this line may not format correctly
	XBR0_Init();
	Interrupt_Init();
	PCA_Init();
	SMB_Init();

	while (1)
	{ 
		if(r_count>=4)
		{
			r_count=0;
			ReadRanger();
			printf("Light: %u \n\r", light);
			PingRanger();
		}
	}


}
//-----------------------------------------------------------------------------
// ReadRanger
//-----------------------------------------------------------------------------
//
// Read Ranger Light Data
//
unsigned int ReadRanger()
{
	i2c_read_data(addr, 1, Data, 1); // read two bytes, starting at reg 2;
	light=Data[0];
	return light;
}

//-----------------------------------------------------------------------------
// PingRanger
//-----------------------------------------------------------------------------
//
// Determine min and max pulsewidth by user input
//
void PingRanger(void)
{
	Data[0] = 0x51;
	i2c_write_data(addr, 0, Data, 1); // write one byte of data to reg 0 at addr
}
//-----------------------------------------------------------------------------
// Interrupt_Init
//-----------------------------------------------------------------------------
//
// Enable interupts
//
void Interrupt_Init(void)
{
	// IE and EIE1
	EA=1;
	EIE1 |= 0x08;
}
//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init(void)
{
	XBR0 = 0x27; //configure crossbar with UART, SPI, SMBus, and CEX channels as
	// in worksheet
}
//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
	// reference to the sample code in Example 4.5 - Pulse Width Modulation implemented using
	// Use a 16 bit counter with SYSCLK/12.
	PCA0MD = 0x81;
	PCA0CPM3 = 0xC2;
	PCA0CN = 0x40; //Enable PCA counter
}
//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR ( void ) __interrupt 9
{
	if(CF)
	{
		CF=0; //clear flag
		PCA0 = 28672;//determine period to 20 ms
		r_count++;
	}
	PCA0CN &= 0x40; //Handle other interupt sources
}
//-----------------------------------------------------------------------------
// SMB_Init
//-----------------------------------------------------------------------------
//
// Set up the I2C Bus
//
void SMB_Init()
{
	SMB0CR = 0x93;	//Sets SCL to 100 kHz (actually ~94594 Hz)
	ENSMB = 1;		//Enables SMB
}
//-----------------------------------------------------------------------------