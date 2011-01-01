//linear-POV-zigzag.c for IMSAInterssion 2011


#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF, LVP = OFF
#include "p18f46K20.h"
#include "intersessionpov.h"
#include <timers.h>

#pragma idata bigdata
//int alphabet[27][3] = {
int alphabet_3x5[28][3] = {
	{ 0b00011110, 0b00101000, 0b00011110 }, //A
	{ 0b00111110, 0b00101010, 0b00010100 }, //B
	{ 0b00011100, 0b00100010, 0b00100010 }, //C
	{ 0b00111110, 0b00100010, 0b00011100 }, //D
	{ 0b00111110, 0b00101010, 0b00101010 }, //E
	{ 0b00111110, 0b00101000, 0b00101000 }, //F
	{ 0b00011100, 0b00101010, 0b00001100 }, //G
	{ 0b00111110, 0b00001000, 0b00111110 }, //H
	{ 0b00000010, 0b00101110, 0b00000010 }, //I
	{ 0b00100000, 0b00100010, 0b00111110 }, //J
	{ 0b00111110, 0b00001000, 0b00010110 }, //K
	{ 0b00111110, 0b00000010, 0b00000010 }, //L
	{ 0b00111110, 0b00011000, 0b00011110 }, //M
	{ 0b00111110, 0b00100000, 0b00011110 }, //N
	{ 0b00011100, 0b00100010, 0b00011100 }, //O
	{ 0b00111110, 0b00101000, 0b00010000 }, //P
	{ 0b00011110, 0b00100110, 0b00011110 }, //Q
	{ 0b00111110, 0b00101000, 0b00010110 }, //R
	{ 0b00010010, 0b00101010, 0b00100100 }, //S
	{ 0b00100000, 0b00111110, 0b00100000 }, //T
	{ 0b00111110, 0b00000010, 0b00111110 }, //U
	{ 0b00111100, 0b00000010, 0b00111100 }, //V
	{ 0b00111100, 0b00001100, 0b00111100 }, //W
	{ 0b00110110, 0b00001000, 0b00110110 }, //X
	{ 0b00110000, 0b00001000, 0b00111110 }, //Y
	{ 0b00100110, 0b00101010, 0b00110010 }, //Z
	{ 0b00000000, 0b00000000, 0b00000000 }, //<space>
	{ 0b00000000, 0b00111010, 0b00000000 } //!
};



void tmr0interrupt(void);

#pragma code high_vector_section=0x8
void
high_vector (void)
{
  _asm GOTO tmr0interrupt _endasm
}
/*
#pragma code low_vector_section=0x18
void
low_vector (void)
{
  _asm GOTO tmr2 _endasm
}
*/
#pragma code

/*
volatile unsigned current_ad_value;
int count = 0;
volatile enum { DIR_LEFT = 0, DIR_RIGHT } direction;
*/

int curAddress=0;
int columnDelay=25;
int displayLength=0; //number of columns to display


#pragma interrupt tmr0interrupt
void
tmr0interrupt (void) {
	/* clear the timer interrupt flag */
	PIR1bits.TMR0IF = 0;
	
	//update column
	LATB=~readEEPROM(curAddress);
	
	//reset timer
	resetColumnTimer();
}



void main (void) {
	TRISB = 0;
	LATB = 0xFF;

	//setup
	displayAddString("Hello");
	columnDelay=25; //can also adjust TS0 prescaler

	OpenTimer0(TIMER_INT_ON &
				T0_8BIT & //T0_16BIT
				T0_SOURCE_INT &
				T0_PS_1_1); //T0_PS_1_2,4,16,...256

	resetColumnTimer();

	curAddress=0;
	//loop
	while (1) {
		
	}
}

void resetColumnTimer() {
	WriteTimer0(255-columnDelay);
}

void displayAddString(char theString[]) {
	char* curChar = theString;
	while(*curChar != '\0'){
	    displayAddChar(*curChar);
	    curChar++;
	}
	/*
	int i;
	// char* curChar;
	// curChar = theString;
	for(i=0; i++; i<sizeof(theString)) {
		displayAddChar(theString[i]);
	}
	*/
}

void displayAddChar(char theChar) {
	int alphIdx=theChar-97;
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][0]);
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][1]);
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][2]);
	writeEEPROM(curAddress, 0b00000000); //space after letter
	displayLength=displayLength+4;
}

void main_old (void) {
	// We will be shifting 0's in from a side to move the 1 (lit LED)
	char pattern = 0b00000001;
	char shiftDir = 0;	//1 RSHIFT, 0 LSHIFT
	
	// Set TRISB to configure PORTB as OUTPUTS
	TRISB = 0;
	
	// Configure the LATB register to set all PORTB outputs HIGH
	// Remember: We are grounding the LEDs into the PORT pins.
	// The LED will turn on when the pin LATB OUPUT is LOW
	LATB = 0xFF;
	while (1) {
		// Delay some cycles between each LED change
		Delay1KTCYx(5);
		
		// We are at one end of the LED string, switch directions
		if(pattern == 0b1000000) {
			// Change shift direction
			shiftDir = 1;
		}
		// We are at the other end, switch directions
		else if(pattern == 0b00000001) {
			//Change shift direction
			shiftDir = 0;
		}
		
		if(shiftDir == 0){
			//Left shift in a 1
			pattern = pattern << 1;}
		else { //if (shiftDir == 1){
			//Right shift in a 1
			pattern = pattern >> 1;
		}
		
		// Set the IO pin Latches
		// We need to invert the pattern bit string since
		// the LEDs are wired to light when LATB is LOW
		LATB = ~pattern;
	}
}

int readEEPROMin(int address) {
	// *** enable/disable interrupts
	EECON1bits.EEPGD = 0;  /* READ step #1 */
	EEADR = address;             /* READ step #2 */
	EECON1bits.RD = 1;     /* READ step #3 */
	curAddress=address++;
	return EEDATA;    /* READ step #4 */
}

void writeEEPROM(int address, int value) {
	// *** enable/disable interrupts	
	EECON1bits.EEPGD = 0;  /* WRITE step #1 */
	EECON1bits.WREN = 1;   /* WRITE step #2 */
	EEADR = address;             /* WRITE step #3 */
	EEDATA = value;    /* WRITE step #4 */
	EECON2 = 0x55;         /* WRITE step #5 */
	EECON2 = 0xaa;         /* WRITE step #6 */
	EECON1bits.WR = 1;     /* WRITE step #7 */
	while (!PIR2bits.EEIF) /* WRITE step #8 */
	  ;
	PIR2bits.EEIF = 0;     /* WRITE step #9 */

	curAddress=address++;
}