#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
#include "clock.h"
#include "uart0.h"
#include "motor.h"
#include "pedometer.h"
#include "tm4c123gh6pm.h"
#include "commandInput.h"


typedef struct node
{
    int8_t command;
    int8_t subCommand;
    int32_t argument;
}node;

/*
 * Read instructions from lab5
 * if instruction is forward, reverse, CW, CCW, wait, pause, stop
 *      create a node and add it to the linked list.
 * else if instruction is list, delete, insert
 */
//PORTF MASKS
#define SW2_MASK 16

#define DEBUG
#define MAXCOMMANDS 20
#define CMDNUMBER 12

//#define NULL ((void *)0)
const char commands[CMDNUMBER][10] = {"forward", "reverse", "CW", "CCW", "wait", "pause", "stop", "list", "delete", "run", "insert"};
const int commandNums[CMDNUMBER] = {0,1,2,3,4,5,6,7,8,9,10,11};
node commandList[MAXCOMMANDS];
int commandCount;
int waitArgument; //0 = pb, 1 = distance

void initPushButton()
{
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure pushbutton pin
    GPIO_PORTF_DIR_R &= ~SW2_MASK;
    GPIO_PORTF_DEN_R |= SW2_MASK;
    GPIO_PORTF_PUR_R |= SW2_MASK;
}

void waitPushButton()
{
    char out[50];
    while(GPIO_PORTF_DATA_R & SW2_MASK);

}

void runCommands()
{
    char out[50];
    uint8_t i;
    for(i = 0; i < MAXCOMMANDS; i++)
    {
        switch(commandList[i].command)
        {
            case 0:
            {
                //call normal forward function
                SLEEP = 1;
                forward(commandList[i].argument, 0, 0);
                SLEEP = 0;
                break;
            }
            case 1:
            {
                //reverse normal
//                void reverse(uint16_t distance_cm, int sensorDistance, uint8_t mode)
                SLEEP = 1;
                reverse(commandList[i].argument, 0, 0);
                SLEEP = 0;
                break;
            }
            case 2:
            {
                //CW
                SLEEP = 1;
                rotateCW(commandList[i].argument);
                SLEEP = 0;
                break;
            }
            case 3:
            {
                //CCW
                SLEEP = 1;
                rotateCCW(commandList[i].argument);
                SLEEP = 0;
                break;
            }
            case 4:
            {
                //wait pb or distance
                //0 - distance, 1 - pb
                if(commandList[i].subCommand == 1)
                    waitPushButton();
                if(commandList[i].subCommand == 0)
                {
//                    sprintf(out,"argument = %d, subCom = %d \n", commandList[i].argument, commandList[i].subCommand);
//                    putsUart0(out);
                    SLEEP = 1;
                    forward(0xFFFF, commandList[i].argument, 1);
                    SLEEP = 0;
                }
                    break;
            }
            case 5:
            {
                //pause
                SLEEP = 1;
                pauseRobot(commandList[i].argument);
                SLEEP = 0;
                break;
            }
            case 6:
            {
                //stop
                SLEEP = 1;
                stopRobot();
                SLEEP = 0;
                break;
            }
            default:
            {
                break;
            }
        }
    }
}

void deleteCommand(uint8_t line)
{
    //loop over struct array until you find "line"
    //from that point commandList[i] = commandList[i+1]
    //mark commandList[commandList.size() - 1] as empty
        //commandList[commandList.size() - 1].command = -1;
    char out[50];
    if(line > MAXCOMMANDS)
    {
        sprintf(out,"Array size is only %d\n", MAXCOMMANDS);
        putsUart0(out);
    }
    uint8_t i;
    for(i = line; i < MAXCOMMANDS - 1; i++)
    {
        commandList[i] = commandList[i+1];
    }
    commandList[i + 1].command = -1;
}
void insertCommand(uint8_t position)
{
    //using code in main, take in new command to insert
        //returned = command, subCommand, argument;
            //if command > 6, those types of commands cannot be entered
            //if command < 4 && argument == 0
                //argument = 0xFFFF;
            //if command == 6 or 4
                //argument = -1;
    //create new node
    //loop over array and see if there are any open spots
    //if no, "no space"
    //if yes, add node
    //loop starting at "position" and shift everything else left
        //commandList[i] = commandList[i+1];
    //commandList[position] = new node

    char out[50];
    USER_DATA data;
    putsUart0("> ");
    // Get the string from the user
    getsUart0(&data);
    // Echo back to the user of the TTY interface for testing
    #ifdef DEBUG
    putsUart0(data.buffer);
    putcUart0('\n');
    #endif
    // Parse fields
    //parseFields(&data);
    parseFields(&data);
    // Echo back the parsed field data (type and fields)
//    #ifdef DEBUG
//    uint8_t i;
//    for (i = 0; i < data.fieldCount; i++)
//    {
//        putcUart0(data.fieldType[i]);
//        putcUart0('\t');
//        putsUart0(&data.buffer[data.fieldPosition[i]]);
//        putcUart0('\n');
//    }
//    #endif
    // Command evaluation
    // set add, data → add and data are integers
    int argument;
    int sending = -1;
    uint8_t i;
    bool valid = false;
    for(i = 0; i < CMDNUMBER; i++)
    {

        if (isCommand(&data, commands[i], 0))
        {
//                sprintf(out,"C = %s\n", commands[i]);
//                putsUart0(out);

            int subCommand = -1;
            char* command = getFieldString(&data, 0);
            if(commandNums[i] == 4)
            {
                subCommand = getFieldString(&data, 1); //returns 0 when fails
                if(subCommand == 0)
                {
                    subCommand = getFieldInteger(&data, 2);
                    argument = getFieldInteger(&data, 1);
//                    sprintf(out,"command = %s, subCommand = %d, DIST = %d\n", command,argument, subCommand);
//                    putsUart0(out);
                    sending = 1;
                }
                else
                {
                    char * scomd = getFieldString(&data, 1);
                    argument = getFieldInteger(&data, 2);
//                    sprintf(out,"command = %s, subCommand = %s\n", command,scomd);
                    putsUart0(out);
                    sending = 0;
                }

            }
            else
            {
                argument = getFieldInteger(&data, 1);
            }
            valid = true;
            break;
        // do something with this information
        }
    }
    // Process other commands here
    // Look for error
    if (!valid)
        putsUart0("Invalid command\n");

    //if command > 6, those types of commands cannot be entered
    //if command < 4 && argument == 0
        //argument = 0xFFFF;
    //if command == 6 or 4
        //argument = -1;
    if(commandNums[i] > 6)
    {
        sprintf(out,"Command cannot be added to command array");
        putsUart0(out);
    }
    if(commandNums[i] < 4 && argument == 0)
    {
        argument = 0xFFFF;
    }
    if(commandNums[i] == 6 || commandNums[i] == 4)
    {
        argument = -1;
    }

    node newNode;
    newNode.command = commandNums[i];
    newNode.argument = argument;
    newNode.subCommand = sending;

//    sprintf(out,"command = %d, argument = %d, subComd = %d\n", newNode.command, newNode.argument, newNode.subCommand);
//    putsUart0(out);
    bool freeSpace = false;
    for(i = 0; i < MAXCOMMANDS; i++)
    {
        if(commandList[i].command == -1)
        {
            if(i < position)
                position = i;
            freeSpace = true;
            break;
        }
    }

    if(!freeSpace)
    {
            sprintf(out,"Command Array is Full, delete a command");
            putsUart0(out);
            return;
    }
    for(i = MAXCOMMANDS - 1; i > position; i--)
    {
        commandList[i] = commandList[i - 1];
    }
    commandList[position] = newNode;

}

void initializeCommandArray()
{
    uint8_t i;
    for(i = 0; i < MAXCOMMANDS; i++)
    {
        commandList[i].command = -1;
        commandList[i].subCommand = -1;
    }
}
void addNode(int command, int argument, int subCommand)
{
    //if command => 7, do not add it to the queue. Just execute it.
    char out[50];

    if(command > 6)
    {
        //{"forward", "reverse", "CW", "CCW", "wait", "pause", "stop", "list", "delete", "run", "insert"}
        if(command == 7)
        {
            //list
            printList();
        }
        else if(command == 8)
        {
            //delete
            deleteCommand(argument - 1);
        }
        else if(command == 9)
        {
            //run
            runCommands();
        }
        else
        {
            //insert
            insertCommand(argument - 1);
        }
    }
    else
    {

        if(commandCount > MAXCOMMANDS - 1)
        {
            sprintf(out,"MAX COMMANDS REACHED\n");
            putsUart0(out);
            return;
        }
//        {"forward", "reverse", "CW", "CCW", "wait", "pause", "stop", "list", "delete", "run", "insert"};
        int tempArgument = argument;

        if(command < 4 && argument == 0)
        {
            argument = 0xFFFF;
        }
        else if(command == 6) // || command == 4
        {
            argument = -1;
//            if (command == 6)
//                argument = -1;
//            else if(command == 4 && subCommand == 1)
//                argument = -1;
        }
        else if(command == 4 && subCommand == 1) //wait pb
        {
            argument = -1;
        }
        uint8_t i;
        for(i = 0; i < MAXCOMMANDS; i++)
        {
            if(commandList[i].command == -1)
            {
                commandList[i].command = command;
                commandList[i].argument = argument;
                if(command == 4)
                {
                    commandList[i].subCommand = subCommand;
                }
                commandCount++;
                break;
            }
        }
    }
}

void printList()
{
    char out[50];

    uint8_t i;
    for(i = 0; i < MAXCOMMANDS; i++)
    {
        if(commandList[i].command != -1)
        {
            if(commandList[i].argument == -1)
            {
                if(commandList[i].command == 4 && commandList[i].subCommand == 1) //1 = wait pb
                {
                    sprintf(out,"%d. %s pb\n",i + 1, commands[commandList[i].command]);
                    putsUart0(out);
                }
                else
                {
                    sprintf(out,"%d. %s\n",i + 1, commands[commandList[i].command]);
                    putsUart0(out);
                }
            }
            else
            {
                if(commandList[i].command == 4 && commandList[i].subCommand == 0)
                {
                    sprintf(out,"%d. %s distance %d\n",i + 1, commands[commandList[i].command],commandList[i].argument);
                    putsUart0(out);
                }
                else
                {
                sprintf(out,"%d. %s %d\n",i + 1, commands[commandList[i].command], commandList[i].argument);
                putsUart0(out);
                }
            }
        }
    }
}

int main(void)
{
    initCommandHw();
    initPushButton();
    initializeCommandArray();
    initSystemClockTo40Mhz();
    initMotor();
    initPedometer();
    char out[50];

    USER_DATA data;

    while(1)
    {
        putsUart0("> ");
        // Get the string from the user
        getsUart0(&data);
        // Echo back to the user of the TTY interface for testing
        #ifdef DEBUG
        putsUart0(data.buffer);
        putcUart0('\n');
        #endif
        // Parse fields
        //parseFields(&data);
        parseFields(&data);
        // Echo back the parsed field data (type and fields)
//        #ifdef DEBUG
        uint8_t i;
//        for (i = 0; i < data.fieldCount; i++)
//        {
//            putcUart0(data.fieldType[i]);
//            putcUart0('\t');
//            putsUart0(&data.buffer[data.fieldPosition[i]]);
//            putcUart0('\n');
//        }
//        #endif
        // Command evaluation
        // set add, data → add and data are integers
        bool valid = false;
        char* command;
        for(i = 0; i < CMDNUMBER; i++)
        {

            if (isCommand(&data, commands[i], 0))
            {
//                sprintf(out,"C = %s\n", commands[i]);
//                putsUart0(out);

                int argument;
                int subCommand = -1;
                int sending = -1;
                command = getFieldString(&data, 0);
                if(commandNums[i] == 4)
                {
                    //0 - distance, 1 - pb
                    //sending = 0 d, 1 pb
                    subCommand = getFieldString(&data, 1); //returns 0 when fails
                    command = getFieldString(&data, 1);
//                    if(subCommand == 0)
//                    {
//                        subCommand = getFieldInteger(&data, 2);
//                        argument = getFieldInteger(&data, 1);
//                        sending = 1;
//                        if(command[0] == 'd')
//                    }
//                    else
//                    {
//                        char * scomd = getFieldString(&data, 1);
//                        argument = getFieldInteger(&data, 2);
//                        sending = 0;
//                    }
                    if(command[0] == 'd')
                        sending = 0;
                    else if(command[0] == 'p')
                        sending = 1;
                    argument = getFieldInteger(&data, 2);

                }
                else
                {
                    argument = getFieldInteger(&data, 1);
//                    sprintf(out,"command = %s, argument = %d\n", command,argument);
//                    putsUart0(out);
                }
//                if(!sending)
                addNode(commandNums[i], argument,sending);
//                else if(sending)
//                    addNode(commandNums[i], subCommand,sending);

                valid = true;
//                printList();
                break;
            // do something with this information
            }
        }
        // alert ON|OFF → alert ON or alert OFF are the expected commands
        if (isCommand(&data, "alert", 1))
        {
            char* str = getFieldString(&data, 1);
            valid = true;
        // process the string with your custom strcmp instruction, then do something
        }
        // Process other commands here
        // Look for error
        if (!valid)
            putsUart0("Invalid command\n");

    }

	return 0;
}
