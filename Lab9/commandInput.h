#ifndef COMMANDINPUT_H
#define COMMANDINPUT_H

// PortF masks
#define GREEN_LED_MASK 8
#define RED_LED_MASK 2


#define MAX_CHARS 80
#define MAX_FIELDS 5

//const char commands[10][10] = {"forward", "reverse", "CW", "CCW", "wait", "pause", "stop"};

typedef struct _USER_DATA
{
    char buffer[MAX_CHARS+1];
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
} USER_DATA;

void  *memset(void *b, int c, int len);
void initCommandHw();
void getsUart0(USER_DATA* data);
int isAlpha(char c);
int isNum(char c);
void parseFields(USER_DATA* data);
bool isCommand(USER_DATA* data, const char strCommand[], uint8_t minArguments);
char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
int getFieldInteger(USER_DATA* data, uint8_t fieldNumber);


#endif
