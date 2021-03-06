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

#define MAX 16

#define ACT    0               // macros for commands in order to save info
#define PRE    1
#define RD     2
#define WR_C   3

#define BGROUP 4               // number of bank groups
#define BANK   4               // number of banks per bank group

#define TRUE 1
#define FALSE 0

#define RC      76
#define RAS     52
#define RRD_L   6
#define RRD_S   4
#define RP      24
#define CWL     24
#define CL      24
#define RCD     24
#define WR      20
#define RTP     12
#define CCD_L   8
#define CCD_S   4
#define BURST   4
#define WTR_L   12
#define WTR_S   4


// Global variable
int DEBUG;  //DEBUG program if 1, otherwise set to 0.
int clk;

// holds the information available to only each request in the queue, important for issuing commands
typedef struct com_info {

    int origin_arrival;                         // contains when the operation was requested by the CPU
    int req_started;                            // to indicated whether the request has been started or now TRUE = 1, FALSE = 0
    int curr_op;                                // operation that's being serviced (RD = 0, WR, = 1, F = 2)
    int prevCommandReq;                         // previous command issued to this request
    int timeLapsedReq;                          // time lapsed since previous command was issued to this request, gets incremented until new command is issued, at which it is reset


}com_info_s;

typedef struct Bit_Field {
    unsigned int byte_offset:3;
    unsigned int lower_col:3;
    unsigned int bank_group:2;
    unsigned int bank:2;
    unsigned int upper_col:8;
    unsigned int row:15;
} hex_field_s; //when writing code, initialize doing 'hex_field_s var_name = blah'

struct node { //For the queue
    int qentry_t; //The time in CPU clock cycles of when item is actually added into the queue
    int counter; //Used to count how long the request has been in the queue
    unsigned long long request_info[3]; //The information from the request is held in this variable (ie time, operation, hexadecimal address)
    com_info_s c_info;         // information used to issue commands is held here
    hex_field_s map;

    struct node *next;
};

// holds the vital general info available to all requests
typedef struct general_info {

    int openPage;                                   // indicates if there is a page open true=1, false=1
    unsigned int page;                              // actual page that is opened
    int precharged;                                 // indicates whether a bank in bank group has been pre-charged, 1 = precharged, 0 = not precharged note: all banks are pre-charged to start
    int t_since_comnd[4];

}general_info_s;

struct node *front = NULL;
struct node *rear = NULL;




//functions
void display();
void enqueue(unsigned long long[], int);
void dequeue();
int readOneLine(FILE *filePointer, unsigned long long request[3]);
int MagicHappensHere(int clk, struct node *front, int queue_size);
hex_field_s address_map(unsigned long long address);
void initialize_stuff(general_info_s infoForALL[BGROUP][BANK]);

general_info_s info4All[BGROUP][BANK];


const char * operation[] = {
        "READ",
        "WRITE",
        "FETCH",
};

 //------------------------------------------------------------------------------------------

int main()
{
    unsigned long long request[3]; //Variable to hold one row/line with info of <time><operation><hexadecimal address>.
    char fileName[50]; //string to hold user's file name input.
    char response[10]; //character to hold user option for debugging
    FILE *filePointer; //Pointer for trace file.
    int rows = 0; //How many rows (or lines) are read from the trace file.

    initialize_stuff(info4All);

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

    while ( readOneLine(filePointer, request) == 0) {
        if (DEBUG) {
            printf("%I64u, %I64u, %I64X\n", request[0], request[1], request[2]);
        }
        rows++;
    }
    if (DEBUG) {
        printf("%d lines read in total!\n", rows);
    }

    fseek(filePointer, 0, SEEK_SET); //Reset file pointer to top of file to read through again.

    //Begin clock clock cycles.
    clk = 0;
    //int request_t = 0;
    int queue_size = 0;
    int pendingReq = 0;
    int end_of_file = 0;
    hex_field_s req_add_field;



    //readOneLine(filePointer, request);
    req_add_field = address_map(0x01FF970AA);
    printf("lower col=[0x%1X], BG=[0x%1X], Bank=[0x%1X], Upper col=[0x%2X], row=[0x%4X]\n", req_add_field.lower_col, req_add_field.bank_group, req_add_field.bank, req_add_field.upper_col, req_add_field.row);
    req_add_field = address_map(0x01FF97080);
    printf("lower col=[0x%1X], BG=[0x%1X], Bank=[0x%1X], Upper col=[0x%2X], row=[0x%4X]\n", req_add_field.lower_col, req_add_field.bank_group, req_add_field.bank, req_add_field.upper_col, req_add_field.row);
    req_add_field = address_map(0x10FFFFF00);
    printf("lower col=[0x%1X], BG=[0x%1X], Bank=[0x%1X], Upper col=[0x%2X], row=[0x%4X]\n", req_add_field.lower_col, req_add_field.bank_group, req_add_field.bank, req_add_field.upper_col, req_add_field.row);

    while (1) {
        if (queue_size < 16) {  //If the queue is not full
            if (pendingReq == 0) { //If no pending request
                if (readOneLine(filePointer, request) == 0) { //Next request is read from file
                    pendingReq = 1;
                }
                else { //EOF is reached
                    end_of_file = 1;
                }
            }
            if (pendingReq == 1) { //If we have a pending request
                if (queue_size == 0) { //If queue is empty
                    if (clk < request[0]) { //If theres a pending request and nothing in the queue, just advance time to request time
                        clk = request[0];
                    }
                }
                if (clk >= request[0]) {
                    printf("Clock:%-4d INSERTED: [%4I64u] [%6s] [%11I64X]\n", clk, request[0], operation[request[1]], request[2] );
                    enqueue(request, clk); //Add the request to the queue
                    queue_size++;
                    pendingReq = 0;
                }
            }
        }
        if (queue_size > 0) { //If queue is not empty

            if ((clk % 2) == 0)             // service requests on each DIMM cycle, 2 clk = 1 DIMM
            {
                queue_size = MagicHappensHere(clk, front, queue_size);
            }
            clk++;
        }
        if (queue_size == 0 && pendingReq == 0 && end_of_file == 1) { //If nothing is in the queue and there is not pending request and you
                                                                      // reached the EOF, then the program is finished running.
            break; //Break out of the while loop
        }
    }

    printf("program complete\n");

    fclose(filePointer);

    return 0;
}

 int readOneLine(FILE *filePointer, unsigned long long request[3]) {
    //This function reads an input file and extracts one line (the next line in the file) everytime it is called.
    // This line is temporarily held in request.
    //The text file has the format <time><operation><hexadecimal address>.
    //The output is a 0 if a line is successfully read, otherwise outputs 1 if EOF is reached.

    char lineValue[30]; //This string will hold a single line at a time in the text file.
    unsigned long long hexNum; //This variable is 64bits to read the hex value from the text file.

        if (fgets(lineValue, 30, filePointer) != NULL) {
            char *token = strtok(lineValue, " "); //Create a pointer called token to hold the individual items in the string array
                                               // from a single line.  This will create 3 separated tokens for time, operation, & hex address.
            int k = 0;  //This variable indicates the column index (are we reading time, operation, or hex address?).
            while (token != NULL) {
                if ( k == 2) { //Special case if we are reading the hex address token.
                    sscanf(token, "%I64X", &hexNum);
                    request[k++] = hexNum;
                }
                else {
                    sscanf(token, "%I64u", &request[k++]);
                }
                token = strtok(NULL, " ");
            }
            return 0; //One line in the file was successfully read
        }
        else {
            return 1; //No more lines in the file to read
        }
 }

 int MagicHappensHere(int clk, struct node *front, int queue_size) {
    struct node *temp;
    temp = front;

    while (temp != NULL)
    {

        if((temp->c_info.req_started) == FALSE) // if this request hasn't been started
        {
            if(temp->c_info.curr_op == 0)       // check if mem operation is a READ
            {
                for(int i = 0; i < BGROUP; i++)
                {
                    for(int j = 0; j < BANK; j++)
                    {
                        if(info4All[i][j].openPage == temp->map.row)
                        {

                        }
                    }
                }
            }

        }
        if (1) {
        printf("Clock:%-4d  REMOVED: [%4I64u] [%6s] [%11I64X]\n", clk, temp->request_info[0], operation[temp->request_info[1]], temp->request_info[2] );
        dequeue();
        queue_size--;
        //temp->counter = 0;
        }
        else {
            temp->counter++;
        }
        temp = temp->next;
    }

    return queue_size;
 }

void initialize_stuff(general_info_s infoForALL[BGROUP][BANK])
{

     for(int i=0; i < BGROUP; i++)              // init firstAccess array to all 1's (all banks are precharged to begin)
    {
      for(int j=0; j < BANK; j++)
      {
          infoForALL[i][j].openPage = 0;                                    // no previous bank group to which a command was issued
          infoForALL[i][j].page = 0;                                        // no previous command issued
          infoForALL[i][j].precharged = 1;                                  // time lapsed since last command == 0
          infoForALL[i][j].t_since_comnd[ACT] = 0;                          // no previous bank at the start of program
          infoForALL[i][j].t_since_comnd[WR_C] = 0;
          infoForALL[i][j].t_since_comnd[RD] = 0;
          infoForALL[i][j].t_since_comnd[PRE] = 0;
      }
    }
}

 hex_field_s address_map(unsigned long long address) {
    hex_field_s field;

    field.byte_offset =       (address) & 0x7;
    field.lower_col   =  (address >> 3) & 0x7;
    field.bank_group  =  (address >> 6) & 0x3;
    field.bank        =  (address >> 8) & 0x3;
    field.upper_col   = (address >> 10) & 0xFF;
    field.row         = (address >> 18) & 0x7FFF;

    return field;
 }

 void enqueue(unsigned long long request[3], int clk)
{
    struct node *nptr = malloc(sizeof(struct node));
    nptr->qentry_t = clk; //Remember what CPU clock time the request was actually inserted into the queue
    nptr->counter = 0; //Set amount of time request has been in the queue to 0 intially
    //Hold a copy of the request's information (ie time, operation, hexadecimal address)
    nptr->request_info[0] = request[0];
    nptr->request_info[1] = request[1];
    nptr->request_info[2] = request[2];

    nptr->c_info.curr_op = request[1];      // fill what type of request it is read, write, fetch
    nptr->c_info.origin_arrival = clk;      // fill when it was requested by CPU
    nptr->c_info.prevCommandReq = -1;       // set to -1,(no previous command)
    nptr->c_info.timeLapsedReq = 0;         // set time lapsed since previous command issued to 0
    nptr->c_info.req_started = FALSE;       // request has not been started

    nptr->map = address_map(request[2]);    // map address

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
