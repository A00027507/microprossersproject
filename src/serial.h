#include <stdint.h>

void initSerial(void);
void eputchar(char c);
char egetchar(void);
void eputs(char *String);
void printDecimal(int32_t Value);

void serialWrite(const char *str);
void serialWriteNum(uint16_t n);
void serialWriteLine(const char *str);
char serialRead(void);
