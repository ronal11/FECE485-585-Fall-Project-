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

#define FILENAME "input_trace.txt"
//DEBUG program if 1, otherwise set to 0.
#define DEBUG 1

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


int main()
{
    unsigned long long inputData[100][3]; //Variable to hold up to 100 lines/rows of <time><operation><hexadecimal address>.
    FILE *filePointer; //Pointer for trace file.
    int rows = 0; //How many rows (or lines) are read from the trace file.
    int *rowsPtr = &rows;

    if ((filePointer = fopen(FILENAME, "r")) == NULL) {
        printf("ERROR: Could not open input file.\n");
        return(1); //Quit program
    }

    readInputDataFile(filePointer, inputData, rowsPtr);

    if (DEBUG) {
        for(int i=0; i<rows; i++) {
            printf("%I64u, %I64u, %I64X\n", inputData[i][0], inputData[i][1], inputData[i][2]);
        }
    }

    fclose(filePointer);

    return 0;
}
