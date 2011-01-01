#ifndef INTERSESSIONPOV_H
#define INTERSESSIONPOV_H
void displayAddString(char theString[]);
void displayAddChar(char theChar);

int readEEPROMin(int address);
void writeEEPROM(int address, int value);
#endif
