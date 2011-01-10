#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF, LVP = OFF
#include "p18f46K20.h"
#include "intersessionpov.h"
#include <delays.h>
#include <timers.h>

#pragma idata bigdata
// Each character is described by 3 columns of 5 LEDs
//int alphabet[27][3] = {
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

        // Current implementation uses ASCII math to determine
        // lookup table index. Space and ! are not sequential
        // and will not be acceptable.  --sturnfie
        // TODO: Find better lookup solution
        //{ 0b00000000, 0b00000000, 0b00000000 }, //<space>
        //{ 0b00000000, 0b00111010, 0b00000000 } //!

		// Modified lookup function to check to check input to be lowercase ascii
		// If input is not lowercase ascii (97-122), will display character in
		//   font table index 26 (the one after z.  currently a space) --bbaker
};
#pragma idata


void tmr0interrupt(void);

#pragma code high_vector_section=0x8
void
high_vector (void)
{
  _asm GOTO tmr0interrupt _endasm
}
#pragma code


/*
volatile unsigned current_ad_value;
int count = 0;
volatile enum { DIR_LEFT = 0, DIR_RIGHT } direction;
*/

unsigned char curAddress=0;
int columnDelay=25;
int displayLength=0;	//number of columns to display
int reverseDirection=1; // repeat pattern in verse when we hit the end, or start at beginning? 1 for reverse
int direction=0;		//0 is forward through memory


#pragma interrupt tmr0interrupt

void tmr0interrupt (void) {
	/* clear the timer interrupt flag */
    INTCONbits.TMR0IF = 0;

	//LATB=0b00001111;
	//update column
	LATB=~readEEPROM(curAddress);
	
	//check boundary conditions - start over, or play pattern backwards
	if((curAddress==0)||(curAddress==displayLength)) {
		if(reverseDirection) {
			if(direction==0) {
				curAddress=displayLength; //curAddress--;
			} else {
				curAddress=0; //curAddress++;
			}
			direction=~direction;
		} else {
			curAddress=0;
		}
	}

	//reset timer
	resetColumnTimer();
}



void main (void) {
	int readVal;
	char displayString[] = "hiz";

	TRISB = 0;
	LATB = 0xFF;

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


	disableInterrupts();
	//setup
	blinken();
	displayAddString(displayString);
	blinken();
	
	columnDelay=25; //can also adjust TS0 prescaler

	enableInterrupts();

	OpenTimer0(TIMER_INT_ON &
				T0_8BIT & //T0_16BIT
				T0_SOURCE_INT &
				T0_PS_1_2); //T0_PS_1_2,4,16,...256 --use 1:256 prescaler bbaker

	resetColumnTimer();

	curAddress=0;

	blinken();
	while (1) {
		//blinken();
		//readVal=readEEPROM(curAddress);
		LATB=~readEEPROM(curAddress);
		curAddress++;
		//blinken();
		//LATB=~readVal;
		Delay10KTCYx(20);
		LATB=0b10101010;
		Delay10KTCYx(20);
		if(curAddress==displayLength) break;
	}
	
	//LATB=0b10101010;
	//loop
	while (1) {
		// this is the song that never ends,
		// yes it goes on and on my friends
		// some people
		// staaaaaarted singin' it not knowin' what it was
		// and they'll continue singin it forever just because
	}
}

void resetColumnTimer(void) {
	WriteTimer0(255-columnDelay);
}

void displayAddString(char theString[]) {
	char* curChar;
	curChar = theString;
	
	while(*curChar != '\0'){
		//blinken();
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
	//alphIdx=0;
	/*
	LATB=~alphabet_3x5[alphIdx][0];
	Delay10KTCYx(20);
	LATB=~alphabet_3x5[alphIdx][1];
	Delay10KTCYx(20);
	LATB=~alphabet_3x5[alphIdx][2];
	Delay10KTCYx(20);
	*/
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][0]);
	curAddress++;
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][1]);
	curAddress++;
	writeEEPROM(curAddress, alphabet_3x5[alphIdx][2]);
	curAddress++;
	writeEEPROM(curAddress, 0b00000000); //space after letter
	curAddress++;
	
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
    EEDATA = databyte;      // Load EEDATA with byte to be written 
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






int readEEPROM_old(int address) {
	// *** enable/disable interrupts
	if(direction==0) curAddress=address++; else curAddress=address--;
	EECON1bits.EEPGD = 0;  /* READ step #1 */
	EECON1bits.CFGS = 0; //added
	EEADR = address;             /* READ step #2 */
	EECON1bits.RD = 1;     /* READ step #3 */
	return EEDATA;    /* READ step #4 */
}

void writeEEPROM_old(int address, int value) {
	// *** enable/disable interrupts	
	EECON1bits.EEPGD = 0;  /* WRITE step #1 */
	EECON1bits.CFGS = 0; //added
	EECON1bits.WREN = 1;   /* WRITE step #2 */
	EEADR = address;             /* WRITE step #3 */
	EEADRH = 0x00;	//added
	EEDATA = value;    /* WRITE step #4 */
	disableInterrupts();
	EECON2 = 0x55;         /* WRITE step #5 */
	EECON2 = 0xaa;         /* WRITE step #6 */
	EECON1bits.WR = 1;     /* WRITE step #7 */
	enableInterrupts();
	while (!PIR2bits.EEIF) /* WRITE step #8 */
	  ;
	PIR2bits.EEIF = 0;     /* WRITE step #9 */

	curAddress=address++;
}

void disableInterrupts(void) {
	INTCONbits.GIE=0;
}

void enableInterrupts(void) {
	//INTCONbits.GIE=1;
}

void blinken(void) {
	LATB=0b00110011;
	Delay10KTCYx(20);
	LATB=0b11001100;
	Delay10KTCYx(20);
}