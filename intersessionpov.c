#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF, LVP = OFF

#define cycleLength 1000 / 6 //in ms
#define conversion_columDelayToTimerResetVal  250
	// 1 / 64			// 1 timer val / 64 clock cycles (something of a fudge factor)
	//  * 16000000		// 16 million cycles / 1 second
	//  * 1 / 1000		// 1 second / 1000 ms = 
						// * colDelay in ms gives the timer reload value
						// colDelay is calculated as cycleLength / numCols
#define offset -20
#define useExternalTrigger 1
#define reverseDirection 0 // repeat pattern in verse when we hit the end, or start at beginning? 1 for reverse


#include "p18f46K20.h"
#include "intersessionpov.h"
#include <delays.h>
#include <timers.h>



#pragma code

unsigned char curAddress=0;
unsigned int columnDelay;	// the number to milliseconds between columns.
					//  calculated from cycleLength (defined above) and the number
					//  of columns, which needs to be set by the program.
unsigned int timerResetVal;  // the number of timer counts between columns.
					//  used in resetColumnTimer, and calculated
					//  from columnDelay and the conv factor defined above to timer ticks 
unsigned int numCols=0;	//number of columns to display
int direction=0;		//0 is forward through memory






#pragma idata bigdata
// Each character is described by 3 columns of 5 LEDs
unsigned char alphabet_3x5[28][3] = {
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
	{ 0b00000000, 0b00000000, 0b00000000 } //unknown character (currently space)
};





// interrupt!
#pragma code high_vector_section=0x8
void
high_vector (void)
{
  _asm GOTO tmr0interrupt _endasm
}

#pragma interrupt tmr0interrupt

void tmr0interrupt (void) {
    INTCONbits.TMR0IF = 0;
	disableInterrupts();

	advanceLEDs();
	
	//reset timer
	resetColumnTimer();



	enableInterrupts();
}



//main code

void main (void) {
	int readVal;
	char displayString[] = "abcdefghi";
	//jklmnopqr
	//stuvwxyza
	
	TRISB = 0;
	TRISD = 0b00000100; //need D2 as input for position sense
	LATB = 0xFF;
	LATD = 0b11000000; //my test board has two of the LEDs on D7 and D6.  want them off

	displayAddString(displayString);
	
	columnDelay = cycleLength / numCols;
	
	//timerResetVal = 65536 - (conversion_columDelayToTimerResetVal * columnDelay)/3 + offset;
	timerResetVal = 65536 - (conversion_columDelayToTimerResetVal * columnDelay) + offset;
	
	enableInterrupts();

	OpenTimer0(TIMER_INT_ON &
				T0_16BIT & //T0_16BIT
				T0_SOURCE_INT &
				T0_PS_1_1); //T0_PS_1_2,4,16,...256 --use 1:256 prescaler bbaker

	resetColumnTimer();

	curAddress=0;
	
	//timerResetVal = 1000 / 64 * timerResetVal;

	//loop;
	while (1) {
		if(useExternalTrigger && (PORTD & 0b00000100)) {
			//external trigger
			//direction=0;
			curAddress=0;
		}
		//advanceLEDs();
		//Delay1KTCYx(timerResetVal);
		//Delay10KTCYx(20);

		// this is the song that never ends,
		// yes it goes on and on my friends
		// some people
		// staaaaaarted singin' it not knowin' what it was
		// and they'll continue singin it forever just because
	}
}

void advanceLEDs(void) {
	//update column
	if(curAddress <= numCols-1) LATB=~readEEPROM(curAddress);	 else LATB=0xFF;
	
	//check boundary conditions - start over, or play pattern backwards
	if(reverseDirection) {
		if(curAddress==0) {
			direction=0;
		} else if (curAddress==numCols-1) {
			direction=-1;
		}
		if(direction==0) curAddress++; else curAddress--;
	} else {
		if(curAddress==numCols-1) {
			if(!useExternalTrigger) curAddress=0; else curAddress++;
		} else {
			curAddress++;
		}
	}
}

void resetColumnTimer(void) {
	WriteTimer0(timerResetVal);
}




void displayAddString(char theString[]) {
	char* curChar;
	curChar = theString;
	
	while(*curChar != '\0'){
	    displayAddChar(*curChar);
	    curChar++;
	}
}

void displayAddChar(char theChar) {
	char alphIdx;
	int value;
	if ((theChar >= 'a') && (theChar <= 'z')) {
		alphIdx=theChar-97;
	} else {
		alphIdx=26; //blank - a space
	}

	writeEEPROM(curAddress, alphabet_3x5[alphIdx][0]);
	curAddress++;
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][1]);
	curAddress++;
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][2]);
	curAddress++;
	writeEEPROM(curAddress, 0b00000000); //space after letter
	curAddress++;
	
	numCols=numCols+4;
}





unsigned char readEEPROM(unsigned char address) 
{ // reads and returns the EEPROM byte value at the address given 
  // given in "address". 
    EECON1bits.EEPGD = 0;   // Set to access EEPROM memory 
    EECON1bits.CFGS = 0;    // Do not access Config registers 
    EEADR = address;        // Load EEADR with address of location to write. 
    // execute the read 
    EECON1bits.RD = 1;      // Set the RD bit to execute the EEPROM read 
    // The value read is ready the next instruction cycle in EEDATA.  No wait is 
    // needed. 
    
    return EEDATA;
}   

void writeEEPROM(unsigned char address, unsigned char databyte) 
{ // writes the "databyte" value to EEPROM at the address given 
  // location in "address". 
    EECON1bits.EEPGD = 0;   // Set to access EEPROM memory 
    EECON1bits.CFGS = 0;    // Do not access Config registers 
    EEDATA = databyte;      // Load EEDATA with byte to be written l
    EEADR = address;        // Load EEADR with address of location to write. 
    EECON1bits.WREN = 1;    // Enable writing 
     
    disableInterrupts();     // Disable interrupts 
    EECON2 = 0x55;          // Begin Write sequence
    EECON2 = 0xAA; 
    EECON1bits.WR = 1;      // Set WR bit to begin EEPROM write 
    enableInterrupts();     // re-enable interrupts
    while (EECON1bits.WR == 1) 
    {   // wait for write to complete.  
    	;
    }
    EECON1bits.WREN = 0;    // Disable writing as a precaution. 
}

void disableInterrupts(void) {
	INTCONbits.GIE=0;
}

void enableInterrupts(void) {
	INTCONbits.GIE=1;
}

/*
void blinken(void) {
	LATB=0b00110011;
	Delay10KTCYx(20);
	LATB=0b11001100;
	Delay10KTCYx(20);
}
*/












// *************************************************************************************
//   test loops and old main
// *************************************************************************************

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


void testLoops(void) {
	/* Test for Delayxxxx Statements */
	//while(1){
	//	// Lights off
	//	LATB = 0xFF;
	//	//Delay
	//	Delay10KTCYx(20);
	//	//Lights On
	//	LATB = 0x00;
	//	//Delay
	//	Delay10KTCYx(20);
	//	
	//
	//}
	
	/* Test for EEPROM READ/WRITE */
	//while(1){
	//	//Lights off
	//	LATB = 0xFF;
	//	// Write to EEPROM MEMORY
	//	writeEEPROM(10,0x55);
	//	// Give it a moment
	//	Delay100TCYx(10);
	//	// Read the EEPROM Back to LATB (inverted)
	//	LATB =~readEEPROM(10);
	//	// Give it a long moment
	//	Delay10KTCYx(10);
	//}
}
