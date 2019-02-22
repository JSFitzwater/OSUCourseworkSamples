
/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *
 *  Assignment No. 2  --  'buildrooms'
 *  Class: CS 344.400 
 *  Date: October 27, 2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>  
#include <time.h>  
#include <ctype.h>  
#include <fcntl.h>

//#define PATH_MAX 250


/********************************************************************
** Struct: Room
** Elements:
**   name - file title
**   typeName - kind of room
**   numAdjacent - tally of neighbor rooms (eg, # doors)
**   roomType - int, designate room classification
**   adjacent - (Room) array, ptrs to neighbor rooms (eg, the doors)
********************************************************************/
struct Room { 
    char* name; 
    char* typeName; 
 
    int numAdjacent; 
    int roomType;  

	struct Room** adjacent;
}; 
typedef struct Room Room; 


/********************************************************************
** Struct: Map
** Elements:
**   numDoors - total, room-to-room links
**   numRooms - total, rooms built (prog makes 7)
**   numSteps - tally, inter-room moves by user 
**   numVisited - tally, num of unique rooms visited
**   current - (Room) ptr, maintain user's current room location
**   roomSet - (Room) array, ptrs to 7 rooms on map
**   visited - (Room) array, ptrs, record of unique rooms visited
********************************************************************/
struct Map { 
    int numDoors;
    int numRooms; 
    int numSteps; 
    int numVisited;  
 
    struct Room* current; 
 
    struct Room* roomSet;
    struct Room* visited;  
}; 
typedef struct Map Map; 



/********************************************************************
** fWriteOut
** Descr: 
********************************************************************/
void fWriteOut( Map* m )
{ 
    int aa, bb, cc = 0; 
    /* pointer, assist room file creation */ 
    FILE*  crFILE;  
    /*  pointers, assist neighbor room interactions  */  
    Room*  rA;
    Room*  rB;
 
    /*  (1) create 7 room files 
     *  (2) pass in traits 
     *  (3) close files                */ 
    for( ; cc < 7; ++cc ){ 
        /*  create+open: File w/ write flag  */ 
		crFILE = fopen( m->roomSet[cc].name, "w" );

        /*  insert: Name (room, not file's)  */ 
        fprintf( crFILE, "ROOM NAME: %s\n", m->roomSet[cc].name ); 

        /*  insert: Neighbors  */ 
        aa=0;
        while(aa != (m->roomSet[cc].numAdjacent)-1 ){ 
            fprintf( crFILE, "CONNECTION %i: %s\n", (aa+1),  
                     m->roomSet[cc].adjacent[aa]->name );
			aa += 1;
        }

        /*  insert: Type       */ 
        fprintf( crFILE, "ROOM TYPE: %s", m->roomSet[cc].typeName ); 

        fclose( crFILE ); 
    }

    /*  free temporary memory  */ 
    cc=0;
    for( ; cc < 7; ){ 
        free( m->roomSet[cc].adjacent );
        cc += 1;
    }

    free( m->roomSet );
} 


/********************************************************************
** initAdjacent
** Descr: test roomA's list of neighbors against roomB for match.
**        1 - True. Test-case is neighbor.
**        0 - False. No match found.
********************************************************************/
int initAdjacent( Room* roomA, Room* roomB ) 
{  
    int b;

    for( b=0; b < roomA->numAdjacent; ++b )
    { 
        if( roomA->adjacent[b] == roomB )
        { 
            return 1;
        }
    }
    return 0;
}  


/********************************************************************
** initNumDoors
** Descr: determine number of doors total that'll connect map's  
**        seven rooms. Minimum number of doors (edges) to room: 3; 
**        doors max/room: 6. Any two rooms share strictly one door  
**        (2-share-1). Total doors range: 11 to 21. 
** Do/while: prevent assignment of too many or few total doors (rand
**           allocate number of doors for map; check if value less
**           than 18 or greater than 64).
********************************************************************/
void initNumDoors( Map* m )
{ 
	int ee;
	ee = rand(); 
	do{ 
        ee-=10;    
    }while( ee>21 || ee<11 ); 
	
	m->numDoors = ee; 
}  


/********************************************************************
** mixnameorder
** Descr: 
********************************************************************/
void mixnameorder( int* nameOrder, int* pool )
{
    int idx, good, bb;
	good = 0;       
	bb = 0;       

    /* randomized order  */ 
    while( bb < 7 ){  

            idx = rand()%9;

            if( idx > -1 && idx < 10 && pool[idx] != -1 ){ 
                nameOrder[bb] = pool[idx]; 
                pool[idx] = -1; 
                bb += 1; 
            }  
 
    } 
} 


/********************************************************************
** initRooms
** Descr: initialize 7 prototype rooms
********************************************************************/
void initRooms( Map* m ) 
{
    int ii, kk;
    int pool[10]     = { 0,1,2,3,4,5,6,7,8,9 };
    int nameOrder[7] = { 0,0,0,0,0,0,0 }; 

    char* arrayNames[10] = { "rmAA", "rmBB", "rmCC", "rmDD", 
                             "rmEE", "rmFF", "rmGG", "rmHH", 
                             "rmII", "rmJJ" }; 
    char* arrayType[3] =   { "STARROOM", "MID_ROOM", "END_ROOM" };
    mixnameorder( nameOrder, pool );

    /*  initialize 7 prototype rooms  */ 
    m->roomSet = (Room*) malloc(m->numRooms * sizeof(Room));
	
    for( ii=0; ii < m->numRooms; ++ii )
    {  
        kk = nameOrder[ii];   
            /* ---- */ 
        m->roomSet[ii].name        = arrayNames[kk];
        m->roomSet[ii].adjacent    = (Room**) malloc(sizeof(Room*));
        m->roomSet[ii].numAdjacent = 0 ; 
        m->roomSet[ii].roomType    = 0 ; 
        m->roomSet[ii].typeName    = arrayType[1]; /* default's mid_room */ 
    } 
} 


/********************************************************************
** rdHlp
** Descr: 
********************************************************************/
int rdHlp( ) 
{
    int aa;
	
	aa = rand(); 
	while( aa>12 ){ aa/=2; } 
    aa-=6; 
    if( aa<0 ){ aa*=-1; } 
	return aa; 
} 


/********************************************************************
** createProto
** Descr: 
********************************************************************/
void createProto( Map* m ) 
{ 
    struct Room* rD;
    struct Room* rK;
	int aa = 0; 
	int cc = 0; 
    char* arrayTypes[3]={ "STARROOM", "MID_ROOM", "END_ROOM" }; 
 
  
    m->numRooms = 7;			/* resulting number of Rooms */ 
    initRooms( m ); 
    initNumDoors( m ); 

	
    /* Assign room pairs to doors  */ 
   for( ; cc < m->numDoors; ++cc ){ 
        /*  Choose 2 Rooms to Connect 
		 *  
         *  (1) 1ST ROOM    
         *  rD  Prevent too many doorways created for one room:   
         *  Do: Rand() select one of the seven rooms to assign 
         *  this 'for'-cycle door. Check: if room already has 6 
         *  neighbors, select another room.                         
         *                      AND 
         *  (2) 2ND ROOM   
         *  rK  Prevent overwriting same room or already-constructed 
         *  neighbor relationship. Check: for A & B same struct  
         *  or structs already edge-connected. Do: select another 
         *  room to connect.                                       */ 
        do{
            /* slot1 gets a room, slot2 copies */ 
            rD = &m->roomSet[rdHlp()];
			rK = &m->roomSet[rdHlp()];                                     /**  NOT TOO COMFORTABLE ABOUT SWAPPING OUT rK = rD  **/

            if( rD->numAdjacent == 6 ){ 
                continue; 
            }
            else{ 
            /** rK = &m->roomSet[rdHlp()]; **/                                 /**  NOT TOO COMFORTABLE ABOUT SWAPPING OUT rK = rD  **/
                /* assign slot2 Not-1's room (preventing conflict)
                 * so long as slots same or Already been assigned
                 * connection to one another, slot2 refreshes     */ 
                while( rK==rD || rK->numAdjacent==6 
                              || initAdjacent(rD,rK) ){ 

                    rK = &m->roomSet[rdHlp()];
                }
            } 
        /* if slot1's room already has maximum 
         * neighbor (adjacency) assignments, do again  */
        } while(rD->numAdjacent == 6); 


        /*  (3) MAKE DOOR    
         *  (two-way edge between vertices.)    */ 

        /*  +1 - Increment slot1 & slot2's neighbor tallies.  */
        rD->numAdjacent++;
        rK->numAdjacent++;
     
        /*  Increase temp memory for new connection: 
         *   (neighbors) x (size-of-room)                     */ 
        rD->adjacent = (Room**)realloc(rD->adjacent,
					   sizeof(Room*) * rD->numAdjacent);
        rK->adjacent = (Room**)realloc(rK->adjacent,
					   sizeof(Room*) * rK->numAdjacent);
     
        /*  Add to neighbor's array.            */ 
        rD->adjacent[rD->numAdjacent-1] = rK; 
        rK->adjacent[rK->numAdjacent-1] = rD; 
    }


    /*  (4) STARROOM & END_ROOM  
     *  edit designations of the randomized set of rooms, 
     *  assigning the 'STARROOM' & 'END_ROOM' traits     */ 
    m->roomSet[0].roomType = 1;
    m->roomSet[6].roomType = 2; 
    m->roomSet[0].typeName = arrayTypes[0];
    m->roomSet[6].typeName = arrayTypes[2]; 
} 


/********************************************************************
** main
********************************************************************/
int main() 
{ 
    int fPID;
    Map m;
    char dirName[50];

    srand( 4576 );     		    /* seeded */ 


    /*  make map/rooms layout then make 7 files therefrom  */ 
	
    createProto( &m );
	
    /*  sprintf syntax reference: 
     *   Robbins, Kay A. and Steve Robbins. UNIX Systems 
     *   Programming: Communication, Concurrency, and 
     *   Threads. Upper Saddle River, NJ: Pearson, 2003. 489.  */
 
	sprintf( dirName, "fitzwatj.rooms.%ld", (long)getpid() );

    /*  make directory then open location  
     *  Source reference: 
     *   Michael Kerrisk's host of man pages: 
     *   http://man7.org/linux/man-pages/
     *   man2/stat.2.html                    */ 

    if( mkdir( dirName, 0777 ) == -1 )
    {
        printf( "mkdir error buildrooms\n" );
    } 
 
    if( chdir( dirName ) == -1 )
    {
        printf( "chdir error buildrooms\n" );
    }

    fWriteOut( &m ); 

    /*  done. exit 'buildrooms' instructions  */ 
	
    exit( EXIT_SUCCESS );
} 