#ifndef INTERSESSIONPOV_H
#define INTERSESSIONPOV_H
void displayAddString(char theString[]);
void displayAddChar(char theChar);
void resetColumnTimer(void);

//int readEEPROM(int address);
//void writeEEPROM(int address, int value);
unsigned char readEEPROM(unsigned char address);
void writeEEPROM(unsigned char address, unsigned char databyte); 
void disableInterrupts(void);
void enableInterrupts(void);

void blinken(void);
void advanceLEDs(void);

void tmr0interrupt(void);
#endif
