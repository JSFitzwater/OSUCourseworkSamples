
/*  Name:  James Fitzwater
*  Email: fitzwatj@onid.oregonstate.edu
*
*  Assignment No. 2  --  'adventure'
*  Class: CS 344.400
*  Date: October 27, 2017
*/


#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stddef.h>

#ifndef NULL
#define NULL   ((void *) 0)
#endif



/********************************************************************
** Struct: Room
** Elements:
**   name - file title
**   typeName - kind of room
**   isVisited - flag, when room visited
**   maxAllowed - num of neighbor rooms permitted
**   numAdjcnt - count of neighbor rooms (eg, # doors)
**   roomType - int, designate room classification
**   adjacent - (Room) array, ptrs to neighbor rooms (eg, the doors)
********************************************************************/
struct Room {
    char name[5];
    char typeName[9];

    int isVisited;
    int maxAllowed;
    int numAdjcnt;
    int roomType;

    struct Room** adjcnt;
};

// forward define structure
typedef struct Room Room;



/********************************************************************
** Struct: Map
** Elements:
**   crn -  int, designate current room (helper int for roomSet[i])
**   numDoors - total, room-to-room links
**   numRooms - total, rooms built (prog makes 7)
**   startNum -  int, designate initial room (helper int for roomSet[i])
**   steps - tally, inter-room moves by user
**   vstd - tally, num of unique rooms visited
**   ordrVstd - (Room) array, ptrs, record of unique rooms visited
**   current - (Room) ptr, maintain user's current room location
**   roomSet - (Room) array, ptrs to 7 rooms on map
**   start -  (Room) ptr, initial room (helper ptr for SimulateMap())
********************************************************************/
struct Map {
    int crn;
    int numDoors;
    int numRooms;
    int startNum;
    int steps;
    int vstd;

    int ordrVstd[7];

    struct Room* current;
    struct Room* roomSet;
    struct Room* start;
};

// forward define structure
typedef struct Map Map;



/**********************************************************
** printEnd
** Caller: simulateMap
** Calls : none
** Descr : at end-of-game, prints congratulations & order 
**           of rooms player visited
** IN    : pointer to 'Map' structure
** OUT   : none
***********************************************************/
void printEnd(Map *g)
{
    int x,z;

    printf( "YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n"
            "YOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", 
            g->steps);

    for(x = 0; x < 7; x++)
    {
        z = g->ordrVstd[x];

        if(z != -1)
        {
            printf(" %s\n", g->roomSet[z].name);
        }
    }
}



/**********************************************************
** printMap
** Caller: simulateMap
** Calls : none
** Descr : prints player's current room placement & prompt
**           for next move
** IN    : pointer to 'Map' structure
** OUT   : none
***********************************************************/
void printMap(Map *g)
{
    int i, j;
    Room* currRoom;

    /* print the room set */

    currRoom = g->current;
    printf("\nCURRENT LOCATION: %s \n%d CONNECTIONS: \n", currRoom->name, currRoom->numAdjcnt);
    for(j=0; j<currRoom->numAdjcnt; ++j)
    {
        printf("  %s\n", currRoom->adjcnt[j]->name);
    }

    printf("WHERE TO? -  ");
}



/**********************************************************
** auditLine
** Caller: movesRoom, finalizeMap, markStartRoom, 
**           setupRoomDoors, setupRoomLabels
** Calls : none
** Descr : checks character-by-character two arrays, & ret-
**           urns outcome of audit
** IN    : pointers to char-arrays; int signal for number
**           chars to compare
** OUT   : int outcome of audit
***********************************************************/
int auditLine(char *ln, char *str, int strlength)
{
    int n1;
    int matchRes = -1;

    char *c1 = ln;
    char *c2 = str;

    n1 = 0;
    if (c1[0] == c2[0])
    {
        matchRes = 1;
    }
    for (n1 = 1; n1<strlength; n1++)
    {
        if (c1[n1] == ':' || c2[n1] == ':')
        {
            return matchRes;
        }
        if (c1[n1] != c2[n1])
        {
            matchRes = -1;
        }
    }

    return matchRes;
}



/**********************************************************
** splitLine
** Caller: setupRoomDoors, setupRoomLabels, simulateMap
** Calls : none
** Descr : splices a buffer input in compliance to charac-
**           ter array passed-in
** IN    : pointers to char-arrays; int signal for index of 
**           last letter place to check
** OUT   : none
***********************************************************/
void splitLine(char *ln, char* spln, int endpt)
{
    int n1, n2, n3;
    char *s1 = ln;


    n1 = n2 = n3 = 0;

    while (n1 < endpt)
    {
        if (s1[n1] == ' ') // on <space>
        {
            n1++;    //end loop run
            n2++;    //space seen (1-of-2, 2-of-2)
        }
        else if (n2 == 2) // after 2 spaces seen
        {
            spln[n3] = s1[n1];
            n1++;
            n3++;
        }
        else { // on letters betwixt spaces
            n1++;
        }
    }

    spln[n3] = '\0';
}



// statically-initialized mutex
pthread_mutex_t mutx = PTHREAD_MUTEX_INITIALIZER;


/**********************************************************
** getTime
** Caller: simulateMap
** Calls : none
** Descr : oversees two responsibilities: writing the current 
**           time to a file (currentTime.txt), and reading 
**           this current-time-timestamp from .txt file; this  
**           function incorporates mutex control, locking and  
**           unlocking access to the .txt-file variable
** IN    : int for read-or-write time instruction
** OUT   : none
***********************************************************/
static void* getTime( void* writeOrRead )
{
	int x,y;
    DIR* dr;
    FILE*  crFILE;
    struct dirent *rn;

    time_t t;
    struct tm *loc;
    char buf[200];
    char ln[200];


    // on WRITE instruction
    if( writeOrRead == 0 )
    {
        /* Supplemental Resource:                      *
         *                                             *
         * Kerrisk, Michael. "Time." Secs. 2-4, Chap.  *
         * 10 of The Linux Programming Interface:      *
         * A Linux and UNIX System Programming Handbook*
         * San Francisco: No Starch Press, 2010. 187-  *
         * 200.                                        *
         * code sample:  time/show_time.c              *
         *                                             *
         * Kerrisk. "Threads: Thread Synchronization." *
         * Chap. 30 of The Linux Programming Interface.*
         * code samples: threads/thread_incr.c,        *
         *               threads/thread_incr_mutex.c   */

        // using our static mutex, lock the variable
        y = pthread_mutex_lock(&mutx);
        if( y != 0 ){ exit(y); }

        // open directory
        if ((dr = opendir("../")) == NULL)
        {
            printf("currentTime error opendir \".\"\n\n");
            exit(1);
        }

        // open our currentTime file (creating if not present)
        crFILE = fopen( "currentTime.txt", "w" );

        // get current time
        t = time(NULL);

        // reformat current time according to local zone
        loc = localtime(&t);
        if (loc == NULL)
        {  exit(1);  }

        // convert into buffered string our formatted timestamp
        if(strftime(buf, 200, "%A, %d %B %Y, %H:%M:%S %Z", 
                    loc) == 0)
        {  exit(1);  } 

        // print to file our string buffer
        fprintf( crFILE, "  %s\n", buf);

        // close file
        fclose( crFILE );

        // close directory
        closedir(dr);

        // instruct mutex to release lock over our file
        y = pthread_mutex_unlock(&mutx);
        if( y != 0 ){ exit(y); }
    }


    // on READ instruction
    else{

        /* Supplemental Resource:                      *
         *                                             *
         * Kerrisk, Michael. "Time." Secs. 2-4, Chap.  *
         * 10 of The Linux Programming Interface:      *
         * A Linux and UNIX System Programming Handbook*
         * San Francisco: No Starch Press, 2010. 187-  *
         * 200.                                        *
         * code sample:  time/show_time.c              *
         *                                             *
         * Kerrisk. "Threads: Thread Synchronization." *
         * Chap. 30 of The Linux Programming Interface.*
         * code samples: threads/thread_incr.c,        *
         *               threads/thread_incr_mutex.c   */

        // using our static mutex, lock the following 
        // functional instructions (that is, the access 
        // to our file variable below)
        y = pthread_mutex_lock(&mutx);
        if( y != 0 ){ exit(y); }

        // open directory
        if((dr = opendir("../")) == NULL)
        {
            printf("currentTime error opendir \".\"\n\n");
            exit(1);
        }

        // open file
        if((crFILE = fopen("currentTime.txt","r")) != NULL)
        {
            // clear buffer
            for(x = 0; x<200; x++)
            {
                ln[x] = '\0';
            }

            // fill buffer
            while(fgets(ln, sizeof(ln), crFILE) != 0)
            {
                // print to terminal the file's line
                printf( "%s\n", ln);
            }
        }

        // close file
        fclose( crFILE );

        // close directory
        closedir(dr);

        // instruct mutex to release lock over our file
        y = pthread_mutex_unlock(&mutx);
        if( y != 0 ){ exit(y); }
    }
}



/**********************************************************
** moveRooms
** Caller: simulateMap
** Calls : auditLine
** Descr : checks if player's choice matches a valid connect-
**           ion, update Current Room to this choice, & checks
**           if player's already visited this room
** IN    : pointer to structs 'Map'
** OUT   : int outcome of move attempt
***********************************************************/
int moveRooms(Map* g, char* choice)
{
    int seen, value, w, x, y, z;

    Room* temp;


    temp = g->current;
    value = -1;

    for(x = 0; x < temp->numAdjcnt; x++)
    {
        // if current room's neighbor matches input
        if(auditLine(temp->adjcnt[x]->name, choice, 4) == 1)
        {
            // change return value to successful
            value = 1;

            // update current room memory address
            g->current = temp->adjcnt[x];

            // update current room set number
            for(y = 0; y < 7; y++)
            {
                if(auditLine(temp->adjcnt[x]->name, g->roomSet[y].name, 4) == 1)
                {
                    g->crn = y;
                }
            }

            // update steps taken
            g->steps += 1;

            // check if yet visited
            for(y = 0; y < 7; y++)
            {
                w = g->crn;
                z = g->ordrVstd[y];
                // mark if already seen
                if(w == z)
                {
                    // end checking
                    y = 7;
                    break;
                }
                else{
                    // else, if reach open spot && not yet seen
                    if( z == -1 )
                    {
                        // update - visit order
                        g->ordrVstd[y] = w;

                        // update - visits
                        g->vstd += 1;

                        // end checking
                        y = 7;
                    }
                }
            }
        }
    }
    // send back outcome
    return value;
}



/**********************************************************
** simulateMap
** Caller: main
** Calls : moveRooms, printEnd, printMap
** Descr : oversees running game, specifically, getting pla-
**           yer's room choice, attempting to move into that
**           room, & tracking when player's reached the End
** IN    : pointers to structs 'Map'
** OUT   : none
***********************************************************/
void simulateMap(Map *g)
{
    struct Room* crnt;
    crnt = g->current;

    char choice[4];
    int state, mv, n1, rmno, x;

    pthread_t thrOne;
    pthread_t thrTwo;


    rmno = g->crn;


    state = 1;
    /* state eq  '0' - cycle over; game over
     * state eq  '1' - still cycling; move rooms
     * state eq '-1' - still cycling; re-prompt move */

    while (state != 0)
    {
        /*  Game load error                      */
        if (g->current->roomType == "noRmType")
        {
            printf("ERROR - Current Room Type:%s\n"
                   "Fix - Run game in location without "
                   "non-gameroom subfolders\nEXITING\n",
                    g->current->roomType);
            exit(1);
        }

        /*  Game done                            */
        if (g->current->roomType == 3)
        {
            printEnd(g);

            state = 0;
        }


        /*  If user-choice did not match
        *  connections, then notify & reprint
        *  available options                    */
        else if (state == -1)
        {
            printf("HUH? I DON’T UNDERSTAND THAT "
                   "ROOM. TRY AGAIN.\n\n");

            state = 1;
        }

        /*  Else still in Normal Game Cycles:   *
         *  get user room choice, move user,    *
         *  print new options                   */

        /*   - else{ <state is 1>                  *
         *           - print room'n'doors          *
         *           - get input                   *
         *           - try move, ret               *
         *              result (state)             *
         *           - if success, state eq 1      *
         *              if unsuccess, state eq -1  */

        else { /* state eq 1 */

            printMap(g);

            for(n1 = 0; n1 < 4; n1++)
            {
                choice[n1] = '\0';
            }

            scanf("%s", &choice);

            if(auditLine("time", choice, 4) == 1)
            {
                //checkDirectory();

                x = pthread_create( &thrOne, NULL,
                                    getTime, 0 );
                if( x != 0 ){ exit(x); }

                x = pthread_join( thrOne, NULL);
                if( x != 0 ){ exit(x); }

                x = pthread_create( &thrTwo, NULL,
                                    getTime, 1 );
                if( x != 0 ){ exit(x); }

                x = pthread_join( thrTwo, NULL );
                if( x != 0 ){ exit(x); }

            }
            else{
                state = moveRooms(g, choice);
            }
            printf("\n");
        }
    }

    return;
}



/**********************************************************
** finalizeMap
** Caller: newMap
** Calls : auditLine
** Descr : last preperatory steps; mark each room's kind/
**           type, point Current Room to Start Room's add-
**           ress, initialize our rooms visited to one (start
**           room is first); & init our steps taken to zero
** IN    : pointer to structs 'Map'
** OUT   : none
***********************************************************/
void finalizeMap(Map *g)
{
    int n1;


    /* connect 'current' marker to STARROOM
     * so that game can start from correct
     * first room                              */

    for (n1 = 0; n1<7; n1++)
    {
        if(auditLine(g->roomSet[n1].typeName, "STARROOM", 8) == 1)
        {
            g->roomSet[n1].roomType = 1;
        }

        if(auditLine(g->roomSet[n1].typeName, "MID_ROOM", 8) == 1)
        {
            g->roomSet[n1].roomType = 2;
        }

        if(auditLine(g->roomSet[n1].typeName, "END_ROOM", 8) == 1)
        {
            g->roomSet[n1].roomType = 3;
        }
    }


    // set current room at starting room
    g->crn = g->startNum;
    g->current = g->start;

    // record first 'visited' element
    g->vstd += 1;
    g->ordrVstd[0] = g->crn;

    // initialize steps taken (zero)
    g->steps = 0;
}



/**********************************************************
** markStartRoom
** Caller: newMap
** Calls : none
** Descr : cycles thru all the rooms in the roomSet, when 
**           it matches room whose type is Start Room, notes
**           the memory address & roomSet index so we can 
**           call it at start of the game
** IN    : pointer to structs 'Map'
** OUT   : none
***********************************************************/
void markStartRoom(Map *g)
{
    int x;
    Room* curr;

    for(x = 0; x < g->numRooms; x++)
    {
        curr = &g->roomSet[x];
        if(auditLine(curr->typeName, "STARROOM", 8) == 1)
        {
            g->startNum = x;
            g->start = curr;
        }
    }
}



/**********************************************************
** setupDoorway
** Caller: setupRoomDoors
** Calls : none
** Descr : using next open slot in 'adjcnt' array, copies 
**           memory address of the other room, also incre-
**           ments each room's personal neighbor counts
** IN    : pointers to structs 'Map' & two 'Room's
** OUT   : none
***********************************************************/
void setupDoorway(Map* g, Room* one, Room* two)
{

    // incr adjancy tally
    one->numAdjcnt += 1;
    two->numAdjcnt += 1;

    // build the doorway
    one->adjcnt[one->numAdjcnt - 1] = two;
    two->adjcnt[two->numAdjcnt - 1] = one;
}



/**********************************************************
** setupRoomDoors
** Caller: newMap
** Calls : auditLine, splitLine, setupDoorway
** Descr : opens each of the room files in the local direc-
**           tory, keeping track of the room file we're wo-
**           rking on, seeks each line w/ "CONNECTION",
**           copies the room name on that line, & makes a
**           a door from that pair of rooms
** IN    : pointer to structure 'Map'
** OUT   : none
***********************************************************/
void setupRoomDoors(Map* g)
{
    DIR* dr;
    FILE* rf;
    struct dirent *rn;

    int adjID, cntn, n1, n2, n3, rmID, rmsDone, rmSeenTag, rmsTyped, tm;
    char *term;
    char ln[24];
    char spln[24];


    char* fn[10] = { "rmAA", "rmBB", "rmCC", "rmDD", "rmEE",
        "rmFF", "rmGG", "rmHH", "rmII", "rmJJ" };


    Room* rmSeen;

    // open dir
    if ((dr = opendir(".")) == NULL)
    //if ((dr = opendir(fdn)) == NULL)
    {
        printf("ManifestMap error opendir \".\"\n\n");
        exit(EXIT_FAILURE);
    }

    rmsTyped = 0;
    rmsDone = 0;
    adjID = -1;
    rmID = -1;
    cntn = 0;

    while ((rn = readdir(dr)) != NULL)
	{
        if(rn->d_name[0] == '.')
        {
            continue;
        }
        if(rn->d_name[0] == '.' && rn->d_name[1] == '.')
        {
            continue;
        }

        // try 10 filenames for match
        for (n1 = 0; n1 < 10; n1++)
        {

            /* for use of fopens()                       *
             *                                           *
             * Supplemental Resource:                    *
             *                                           *
             * Falstad, Paul. "zsh, the Z shell: utils.c * 
             * - miscellaneous utilities." UTILS.C.      *
             * zsh.sourceforge.net/Arc/source.html       */

            // open file
            if ((rf = fopen(fn[n1], "r")) != NULL)
            {
                // clear buffer
                for (n2 = 0; n2 < 24; n2++)
                {
                    ln[n2] = '\0';
                }

                // fill buffer
                while (fgets(ln, sizeof(ln), rf) != 0)
                {
                    ln[18] = '\0';

                    if (auditLine(&ln[0], "ROOM TYPE", 9) == 1)
                    {
                        continue;
                    }

                    // Get ID of Room; test if line's name
                    if (auditLine(&ln[0], "ROOM NAME:", 10) == 1)
                    {
                        // clear split-line
                        for (n2 = 0; n2 < 24; n2++)
                        {
                            spln[n2] = '\0';
                        }
                        // reset roomID holder (for use)
                        rmID = -1;

                        // split line on spaces
                        splitLine(&ln[0], &spln[0], 15);

                        // check name against each room in set until match
                        for (n2 = 0; n2 < 7; n2++)
                        {
                            // if file's room name matches
                            if (auditLine(&spln[0], g->roomSet[n2].name, 4) == 1)
                            {
                                // keep track which room we're working on
                                rmID = n2;

                                // and, stop checking
                                n2 = 7;
                            }
                        }
                        // found a match, incr rooms done tally
                        rmsDone += 1;
                    }

                    // Get ID of Neighbor; test if line's connection
                    if (auditLine(&ln[0], "CONNECTION", 10) == 1)
                    {
                        // clear split-line
                        for (n2 = 0; n2 < 24; n2++)
                        {
                            spln[n2] = '\0';
                        }
                        // reset adjacentID holder (for use)
                        adjID = -1;

                        // split line on spaces
                        splitLine(&ln[0], &spln[0], 18);
                        for (n2 = 0; n2<g->numRooms; n2++)
                        {
                            // if cnnctn room's name matches
                            if (auditLine(&spln[0], g->roomSet[n2].name, 4) == 1)
                            {
                                // keep track which room we're working on
                                adjID = n2;

                                if(auditLine(g->roomSet[rmID].name, 
                                        g->roomSet[adjID].name, 4) != 1)
                                {
                                    if( g->roomSet[rmID].maxAllowed > 0 )
                                    {
                                        rmSeen = &g->roomSet[rmID];
                                        rmSeenTag = -1;
                                        for(n2 = 0; n2 < rmSeen->numAdjcnt; n2++)
                                        {
                                            if(auditLine( rmSeen->adjcnt[n2]->name, 
                                                    g->roomSet[adjID].name, 4) == 1)
                                            {
                                                rmSeenTag = 1;
                                            }
                                        }
                                        if( rmSeenTag == -1 )
                                        { 
                                            // Connect Room+Neighbor: setup Doorway
                                            setupDoorway(g, &g->roomSet[rmID], 
                                                            &g->roomSet[adjID]);
                                            g->roomSet[rmID].maxAllowed -= 1;
                                            g->numDoors += 1;
                                        }
                                    }
                                }
                                // and, stop checking
                                n2 = g->numRooms;

                                // reset IDs
                                adjID = -1;
                            }
                        }
                    }
                    // re-clear buffer
                    for (n2 = 0; n2 < 24; n2++)
                    {
                        ln[n2] = '\0';
                    }
                }
                rmID = -1;
            }
        }
        // close dir
        closedir(dr);
        return;
    }
}



/**********************************************************
** setupRoomLabels
** Caller: newMap
** Calls : auditLine, splitLine
** Descr : opens each of the room files in the local direc-
**           tory, copies over each room file's name & type
** IN    : pointer to structure 'Map'
** OUT   : none
***********************************************************/
void setupRoomLabels(Map* g)
{
    DIR* dr;
    FILE* rf;
    struct dirent *rn;

    int n1, n2, n3, roomsNamed, roomsTyped, tm;
    char* term;
    char ln[24];
    char spln[24];

    char* fn[10] = { "rmAA", "rmBB", "rmCC", "rmDD", "rmEE",
        "rmFF", "rmGG", "rmHH", "rmII", "rmJJ" };

    // open dir
    if ((dr = opendir(".")) == NULL)
    //if ((dr = opendir(fdn)) == NULL)
    {
        printf("ManifestMap error opendir \".\"\n\n");
        exit(EXIT_FAILURE);
    }


    roomsNamed = roomsTyped = 0;

    while ((rn = readdir(dr)) != NULL)
	{
        if(rn->d_name[0] == '.')
        {
            continue;
        }

        if(rn->d_name[0] == '.' && rn->d_name[1] == '.')
        {
            continue;
        }

        // open file
        if ((rf = fopen(rn->d_name, "r")) != NULL)
        {

            // clear buffer
            for (n2 = 0; n2; n2++)
            {
                ln[n2] = '\0';
            }

            // fill buffer
            while (fgets(ln, sizeof(ln), rf) != 0)
            {
                ln[19] = '\0';

                /* altho name precedes type-check,
                in theory, only the last name
                seen, that of the start room's.
                will be what this proto-room sees */

                // if line is name
                if (auditLine(&ln[0], "ROOM NAME:", 10) == 1)
                {
                    // split line on spaces
                    splitLine(&ln[0], &spln[0], 15);

                    // fill array 'name' w/ name
                    for (n3 = 0; n3 < 4; n3++)
                    {
                        g->roomSet[roomsNamed].name[n3] = spln[n3];
                    }
                    g->roomSet[roomsNamed].name[4] = '\0';
                    roomsNamed++;
                }

                // if line is room type
                if (auditLine(&ln[0], "ROOM TYPE:", 10) == 1)
                {
                    // split line on spaces
                    splitLine(&ln[0], &spln[0], 19);

                    // fill room type
                    for (n3 = 0; n3 < 8; n3++)
                    {
                        g->roomSet[roomsTyped].typeName[n3] = spln[n3];
                    }
                    g->roomSet[roomsTyped].typeName[8] = '\0';
                    roomsTyped++;
                }
            }
        }

    }
    // close dir
    closedir(dr);
}



/**********************************************************
** setupRoomMemSpc
** Caller: newMap
** Calls : none
** Descr : initialize the game map's set of seven Room's
** IN    : pointer to structure 'Map'
** OUT   : none
***********************************************************/
void setupRoomMemSpc(Map* g)
{
    int n1, n2;
    char nm[4] = "noNm";
    char ty[8] = "noRmType";

    g->crn = -1;
    g->vstd = 0;
    g->steps = 0;
    g->numDoors = 0;
    g->numRooms = 7;
    g->startNum = -1;

    g->roomSet = (Room*)malloc(g->numRooms * sizeof(Room));

    for (n1 = 0; n1 < g->numRooms; n1++)
    {
        for (n2 = 0; n2 < 4; n2++)
        {
            g->roomSet[n1].name[n2] = nm[n2];
        }
        g->roomSet[n1].name[4] = '\0';

        for (n2 = 0; n2 < 8; n2++)
        {
            g->roomSet[n1].typeName[n2] = ty[n2];
        }
        g->roomSet[n1].typeName[8] = '\0';

        for (n2 = 0; n2 < 7; n2++)
        {
            g->ordrVstd[n2] = -1;
        }

        g->roomSet[n1].maxAllowed = 5;
        g->roomSet[n1].isVisited = 0;
        g->roomSet[n1].roomType = 9;
        g->roomSet[n1].numAdjcnt = 0;
        g->roomSet[n1].adjcnt = (Room**)malloc(5*sizeof(Room*));
    }
}



/**********************************************************
** newMap
** Caller: main
** Calls : setupRoomMemSpc, setupRoomLabels, setupRoomDoors, 
**           markStartRoom, finalizeMap
** Descr : helper, bundling functions responsible for recon-
**           structing map created by buildrooms
** IN    : pointer to structure 'Map'
** OUT   : none
***********************************************************/
void newMap(Map* game)
{
    setupRoomMemSpc(game);
    setupRoomLabels(game);
    setupRoomDoors(game);
    markStartRoom(game);
    finalizeMap(game);
}



/**********************************************************
** mostRecentDir
** Caller: main
** Calls : none
** Descr : updates current directory (by chdir) to most-
**           recent fitzwatj.rooms folder created
** IN    : name of current directory
** OUT   : none
***********************************************************/
void mostRecentDir(const char *currdirecty)
{

	/* KERRISK, pp.283-284, 352-357, 364-365 */
	/* zsh.utils.c */

	int gotStat1 = -1;
	int foo, bar;
	struct dirent *fd;
	struct dirent *fnewest;

	struct stat stat1;
	struct stat stat2;

	char dir[40] = "./";
	size_t dlen = strlen(dir) + 1;

	DIR* dd;

	char* f1;
	char* f2;
	char* pathnw;
	char* folder;

	f1 = NULL;
	f2 = NULL;
	pathnw = NULL;
	folder = NULL;


    // open this current storage location
	if ((dd = opendir(".")) == NULL) 
	{
		printf("MostRecentDir error opendir %s \n\n", currdirecty);
		exit(EXIT_FAILURE);
	}

    // so long as read-directory rets file results...
	while ((fd = readdir(dd)) != NULL)
	{

        // skip this result
		if (fd->d_name[0] == '.')
		{
			continue;
		}

        // skip this result
		if (fd->d_name[0] == "." && fd->d_name[1] == ".")
		{
			continue;
		}

        /* reference supplement:                                  *
         *                                                        *
         * Kernighan, Brian W. and Dennis M. Ritchie. "The UNIX   *
         * System Interface: Example - Listing Directories."      *
         * Chap. 8, sec. 6 in The C Programming Language. 2nd ed. *
         * Englewood Cliffs, NJ: Prentice Hall, 1988. 146-149.    */

        // looking for our first directory
        if(gotStat1 == -1)
        {
            stat(fd->d_name, &stat1);

            // found our first directory
            if((stat1.st_mode & S_IFMT) == S_IFDIR)
            {
                // this is the most recent directory
                // (b/c it's only dir we've seen)...
                // so fnewest copies this
                fnewest = fd;

                // and, we found a dir, so update gotStat1
                gotStat1 = 1;
            }
        }

        // ops when we have 1 directory, at least
        if(gotStat1 == 1)
        {
            stat(fd->d_name, &stat2);

            // found another directory
            if((stat2.st_mode & S_IFMT) == S_IFDIR)
            {
                // compare last-modified times

                // if stat2 more recent than stat1
                if(stat2.st_mtime < stat1.st_mtime)
                {
                    // update (point) fnewest to this FD
                    fnewest = fd;
                }
            }
        }
	}

    chdir(fnewest->d_name);
}



/**********************************************************
** main
** Caller: none
** Calls : mostRecentDir,  manifestMap,  simulateMap,
**           cleanUp
** IN    : none
** OUT   : none
***********************************************************/
int main(void)
{

	/*  init pointer to map structure  */
	Map gameMap;

    /* directory structure that'll allow  *
     * access to most-recent rooms folder */
    struct dirent* gameFD;

	char currdir[PATH_MAX];

	/*  source referenced:
	*     Kerrisk, Michael. "Directories And Links:
	*     The Current Working Directory of a Process."
	*     Chap. 18 in The Linux Programming Interface.
	*     San Francisco: No Starch Press, 2010. 364, 365. */

	getcwd(currdir, PATH_MAX);  /* Kerrisk, "Remember where we are."  */
	mostRecentDir(".");


    /*  get 7 files & reconstruct rooms from contents  */
    newMap(&gameMap);

    /* run game */
    simulateMap(&gameMap);


    /*  reference:
    *    Kerrisk. Chap. 18, The Linux
    *    ProgrammingInterface. 364, 365.    */

    chdir(currdir);  /*  Kerrisk, "Return to original directory."  */


    /*  exit 'adventure' program  */
    exit(EXIT_SUCCESS);
}
