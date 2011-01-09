#pragma config FOSC = INTIO67
#pragma config WDTEN = OFF, LVP = OFF
#include "p18f46K20.h"
#include "intersessionpov.h"
#include <delays.h>
#include <timers.h>

#pragma idata bigdata
// Each character is described by 3 columns of 5 LEDs
//int alphabet[27][3] = {
char alphabet_3x5[28][3] = {
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
int reverseDirection=1; // repeat pattern in verse when we hit the end, or start at beginning? 1 for reverse
int direction=0; //0 is forward through memory


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
	char displayer[] = "hiz";
	TRISB = 0;
	LATB = 0xFF;

	//LATB = 0x00;

	//setup
	//displayAddString("hello");
	displayAddString(displayer);
	columnDelay=25; //can also adjust TS0 prescaler

	//INTCONbits.GIEH = 1;	//enable interrupts

	OpenTimer0(TIMER_INT_ON &
				T0_8BIT & //T0_16BIT
				T0_SOURCE_INT &
				T0_PS_1_2); //T0_PS_1_2,4,16,...256 --use 1:256 prescaler bbaker

	resetColumnTimer();

	curAddress=0;

	while (1) {
		//readVal=readEEPROM(curAddress);
		LATB=~readEEPROM(curAddress);
		//LATB=~readVal);
		Delay10KTCYx(20);
		LATB=0b10101010;
		Delay10KTCYx(20);
		//if(curAddress==displayLength) break;
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

	LATB=0b00110011;
	Delay10KTCYx(20);
	LATB=0b11001100;
	Delay10KTCYx(20);
	
	while(*curChar != '\0'){
		/*
		LATB=0b00110011;
		Delay10KTCYx(20);
		LATB=0b11001100;
		Delay10KTCYx(20);
		*/
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
	char alphIdx;
	int value;
	/*
	int test1;
	int test2;
	test1 = (105 > 97);
	test2 = (theChar > 'a');
	*/
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
	writeEEPROM(curAddress, (int) alphabet_3x5[alphIdx][0]);
	writeEEPROM(curAddress, (int) alphabet_3x5[alphIdx][1]);
	writeEEPROM(curAddress, (int) alphabet_3x5[alphIdx][2]);
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

int readEEPROM(int address) {
	// *** enable/disable interrupts
	if(direction==0) curAddress=address++; else curAddress=address--;
	EECON1bits.EEPGD = 0;  /* READ step #1 */
	EECON1bits.CFGS = 0; //added
	EEADR = address;             /* READ step #2 */
	EECON1bits.RD = 1;     /* READ step #3 */
	return EEDATA;    /* READ step #4 */
}

void writeEEPROM(int address, int value) {
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