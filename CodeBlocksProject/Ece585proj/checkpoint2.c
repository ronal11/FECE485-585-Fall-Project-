/**
 * checkpoint1.c - Check point 1 for ECE585 project
 *
 * Name: Team 8 - Jonathan S, Dang N, Ronaldo L, Mousa A
 * Date:   October, 2021
 *
 * Checkpoint 1: Working code to read and parse an input trace file and displaying the parsed values.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// Global variable
int DEBUG;  //DEBUG program if 1, otherwise set to 0.
int clk;

//For the queue------------------------------------------------------------------------------------------
struct node
{
    int removal_t; //The time in CPU clock cycles of when item is removed from queue
    int row; //The line index from the file (to identify which memory request).
//     DATA data;
    struct node *next;
};

struct node *front = NULL;
struct node *rear = NULL;

void display();
void enqueue(int,int);
void dequeue();
//------------------------------------------------------------------------------------------

//For the trace file array information
 void readInputDataFile(FILE *filePointer, unsigned long long inputData[100][3], int *rowsPtr);
 //------------------------------------------------------------------------------------------

int main()
{
    unsigned long long inputData[100][3]; //Variable to hold up to 100 lines/rows of <time><operation><hexadecimal address>.
    char fileName[50]; //string to hold user's file name input.
    char response[10]; //character to hold user option for debugging
    FILE *filePointer; //Pointer for trace file.
    int rows = 0; //How many rows (or lines) are read from the trace file.
    int *rowsPtr = &rows;

    //Ask for file name and store it in variable fileName.
    printf("\nEnter name of data file to read (with extension): ");
    if (fgets(fileName, 50, stdin) != NULL) {
        fileName[strcspn(fileName, "\n")] = 0; //Get rid of new line.
    }

    if ((filePointer = fopen(fileName, "r")) == NULL) {
        printf("ERROR: Could not open input file.\n");
        return(1); //Quit program
    }

    //Ask for user if they want to enable debugging.
    printf("\nRun program with debugging code? (Y/N): ");
    if (fgets(response, 10, stdin) != NULL) {
        response[strcspn(response, "\n")] = 0; //Get rid of new line.
    }

    if (response[0] == 'y' || response[0] == 'Y') {
        DEBUG = 1;
    }
    else if (response[0] == 'n' || response[0] == 'N') {
        DEBUG = 0;
    }
    else {
        printf("ERROR: Invalid debug option response.\n");
        return(2); //Quit
    }

    readInputDataFile(filePointer, inputData, rowsPtr);

    if (DEBUG) {
        for(int i=0; i<rows; i++) {
            printf("%I64u, %I64u, %I64X\n", inputData[i][0], inputData[i][1], inputData[i][2]);
        }
    }

    //Begin clock clock cycles.
    clk = 0;
    int lineIndex = 0;
    int entry_t;
    int queue_size = 0;

    while(1) {
        //Make sure we stop reading from the array of instructions when at the last line from the file.
        //Also make sure queue size is not full (max of 16).
        if (lineIndex < rows) {
            entry_t = inputData[lineIndex][0];
            //Check if its time to execute the next instruction.
            if (clk >= entry_t && queue_size < 16) {
                printf("Entry %d is inserted into queue at time %d\n", lineIndex, clk);
                enqueue(clk, lineIndex);
                lineIndex++;
                clk++;
                queue_size++;
                continue;
            }
        }
        //Check if there is something in the queue.
        if (front != NULL) {
            //Check if it's time to remove an item from the queue.
            if (clk >= front->removal_t) {
                printf("Entry %d is removed from queue at time %d\n", front->row, clk);
                //Check if this is the last instruction in the queue.
                if (front->row == (rows-1)) {
                    dequeue();
                    break; //Exit program.
                }
                else {
                    dequeue();
                    queue_size--;
                    clk++;
                    continue;
                }
            }
        }
        //If its not time to execute anything and nothing is in queue, advance 1 clock cycle.
        clk++;
    }

    printf("program complete\n");

    fclose(filePointer);

    return 0;
}

 void readInputDataFile(FILE *filePointer, unsigned long long inputData[100][3], int *rowsPtr) {
    //This function reads an input file to extract the data in an array called inputData.
    // The text file has the format <time><operation><hexadecimal address>.

    char lineValues[30]; //This string will hold a single line at a time in the text file.
    unsigned long long hexNum; //This variable is 64bits to read the hex value from the text file.

    //Loop every line in the file to extract data until NULL is returned, indicating end of the file.
    int i = 0; //This variable holds how many rows (or lines) we go through.
    while (1) {
        if (fgets(lineValues, 30, filePointer) != NULL) {
            char *token = strtok(lineValues, " "); //Create a pointer called token to hold the individual items in the string array
                                               // from a single line.  This will create 3 separated tokens for time, operation, & hex address.
            int k = 0;  //This variable indicates the column index (are we reading time, operation, or hex address?).
            while (token != NULL) {
                if ( k == 2) { //Special case if we are reading the hex address token.
                    sscanf(token, "%I64X", &hexNum);
                    // printf("lineNum:%I64u.\n", hexNum); //Test
                    inputData[i][k++] = hexNum;
                }
                else {
                    sscanf(token, "%I64u", &inputData[i][k++]);
                }
                token = strtok(NULL, " ");
            }
            i = i + 1; //Increment the row.
        }
        else {
            *rowsPtr = i;
            if (DEBUG) {
                printf("%d lines read in total!\n", *rowsPtr);
            }
            break;
        }
    }
 }

 void enqueue(int timeOfExecution, int row)
{
    struct node *nptr = malloc(sizeof(struct node));
    nptr->removal_t = timeOfExecution + 100;
    nptr->row = row;
    nptr->next = NULL;
    if (rear == NULL)
    {
        front = nptr;
        rear = nptr;
    }
    else
    {
        rear->next = nptr;
        rear = rear->next;
    }
}

void display()
{
    struct node *temp;
    temp = front;
    printf("\n");
    while (temp != NULL)
    {
        //printf("%d\t", temp->data);
        temp = temp->next;
    }
}

void dequeue()
{
    if (front == NULL)
    {
        printf("\n\nqueue is empty \n");
    }
    else
    {
        struct node *temp;
        temp = front;
        front = front->next;
        free(temp);
    }
}
