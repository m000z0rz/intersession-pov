#ifndef INTERSESSIONPOV_H
#define INTERSESSIONPOV_H
void displayAddString(char theString[]);
void displayAddChar(char theChar);
void resetColumnTimer(void);

int readEEPROM(int address);
void writeEEPROM(int address, int value);

void disableInterrupts(void);
void enableInterrupts(void);
#endif
