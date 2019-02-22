
/*  Name:  James Fitzwater
 *  Email: fitzwatj@onid.oregonstate.edu
 *
 *  Assignment No. 3  --  'smallsh'
 *  Class: CS 344.400 
 *  Date: March 05, 2018
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <assert.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

// #ifndef NULL
// #define NULL   ((void *) 0)

// maximizing buffer size, per assignment instructions
#define BUFSZ 2048

/* Definition of task-handling "children," *
 * & forward declaraction of instance "c"  */
struct Child{
          //pid_t* val;   // value of the link
          pid_t  val;   // value of the link
   struct Child* nxt;	// pointer to the next link
   struct Child* prv;	// pointer to the previous link
}; // c;

/* Definition of task-handler wrapper, & *
 * forward declaration of instance "t"   */
struct Tasks{
            int  siz;	// number of links in the deque
   struct Child* cur; 	// pointer to the last link
} t;

/* Definition for word-specific-handling *
 * of user's input                       */
struct INWD{
    char word[2048];
}; typedef struct INWD INWD;

/* Definition for entire-line-handling *
 * of user's input                     */
struct INLN{
    int numWds;
    int maxWds;
    struct INWD* wds;
}; typedef struct INLN INLN;



/* BGDALLOWED
 * In:  flag-switcher trigger (int)
 * Out: flag's new state (int)
 * Desc: function controls whether processes may execute 
 * in the "background." modified by multiplying the current 
 * state with passed-in trigger (1 or -1), a static int 
 * maintains the permission's status.  
 */
int bgdAllowed( int trigger ){
    static int flag = 1;

    flag = (trigger * flag);

    if(flag == -1){ 
        char* msg = "Background Disabled\n";
        write(STDOUT_FILENO, msg, 20);
    }
    else{ 
        char* msg = "Background Enabled\n";
        write(STDOUT_FILENO, msg, 19);
    }

    return flag;
}


/* AUDITLINE
 * In:  two ptrs to char mem addresses
 * Out: result of audit check (int)
 * Desc: personal deployment of "strcmp()," function 
 * compares ASCII value of two words' letters, two 
 * characters at a time, returns "-1" if any comparison
 *  set didn't match, else "1"  
 */
int auditLine( char *check, char *arg ){
    int result, x;
    char *c1; 
    char *c2;

    c1 = check;
    c2 = arg;
    x = 0;
    result = 1;

    // check while both have letters
    while( c1[x] ){
        if (toupper(c1[x]) != toupper(c2[x])){
            result = -1;
        }
        x++;
    }

    return result;
}


/* AUDITFORPID
 * IN:  ptr to char string, ptr to two-char symbol "$$"
 * Out: audit result (int)
 * Descr: a role-defined vers. of "auditline()," 
 * strictly looking for the sybmols "$$" (symbol 
 * for PID expansion) among the command-line arguments, 
 * returning "1" if "true" (an argument has '$$')
 */
int auditForPID( char *check, char *arg ){
    int result1,result2,result3, x;
    char *c1; 
    char *c2;

    c1 = check;
    c2 = arg;
    x = 0;
    result1=result2=result3 = -1;

    // check while both have letters
    while( c2[x] ){
        if((c1[0]) == (c2[x])){
            result1 = 1;
            if(c2[x+1]){
                if((c1[1]) == (c2[x+1])){ 
                    result2 = 1; 
                }
            }
        }
        x++;
    }
    if(result1 && result2 == 1){ result3 = 1; }

    return result3;
}


/* PIDHANDLER
 * In:  ptr to cmd-line input (struct), placemarker (int)
 * Out: none
 * Descr: symbol-expansion "$$" w/ current PID. 
 * copies in the entire cmd-line argument, makes
 *  copy of segment before the "$$" symbol, grabs 
 * the current PID and concates to segment, then 
 * concates the remainder of the cmd-line arguments.
 */
void pidHandler(INLN* ln, int v){

    int x,y,z,mark1,mark2;

    char* orig = ln->wds[v].word;


    char holder[50];
    char pidhold[50];
    for(x=0;x<50;x++){
        holder[x] = '\0';
        pidhold[x] = '\0';
    }
    
    x=0;
    while(orig[x+1]){
        if(orig[x] == 36 && orig[x+1] == 36){
            mark1 = x+1;
            mark2 = x+2;
        }
        x++;
    }

    /* supplemental resource:                 **
    **                                        **
    ** StackOverflow. "How to convert an int  **
    ** to string in C." Answer from user      **
    ** cnicutar. Last modified June 23, 2013. **
    ** http://stackoverflow.com/questions/    **
    ** 8257714/how-to-convert-an-int-to-      **
    ** string-in-c                            */

    sprintf(pidhold, "%d", getpid());


    x=0;
    while(x < mark1){
        holder[x] = orig[x];
        y=x;
        x++;
    }

    x=0;
    while(pidhold[x] != '\0'){
        holder[y+x] = pidhold[x];
        z = y+x;
        x++;
    }

    x=0;
    while(orig[mark2+x]){
        holder[z+x+1] = orig[mark2+x];
        x++;
    }

    x=0;
    while(holder[x] != '\0'){
        ln->wds[v].word[x] = holder[x];
        x++;
    }
}



// *********************************************************************
// **********************  CHILD MNGMT  ********************************
// *********************************************************************


/* ADDTASK
 * In:  process identifying descriptor 
 * Out: none
 * Descr: adds PID to a linked-list library of known tasks
 */
void addTask( pid_t v ) 
{

    // create a link for the value
    struct Child * ch = (struct Child *) malloc(sizeof(struct Child)+1);
    ch->val = (pid_t ) malloc(sizeof(pid_t)+1);
    ch->val = v;

    // check curr capacity
	if( t.siz == 0 ){
        // if q is currently empty, added 
        // link becomes only link in deque
		ch->nxt = ch;
		ch->prv = ch;

        // increase size of deque
        t.siz++;
        // change last to newly added link
        t.cur = ch;
	}
	else{
        // re-allocate pointers
        ch->nxt    = t.cur->nxt;
        ch->prv    = t.cur;
        t.cur->nxt->prv = ch;
        t.cur->nxt = ch;

        // increase size of deque
        t.siz++;
        // change last to newly added link
        t.cur = ch;
	}

}


/* INITTASKLIST
 * In:  none
 * Out: none
 * Descr: initializes the task library's size (number
 *        of tasks) and count of ongoing tasks to zero
 */
void initTaskList() 
{
	t.siz = 0;
	t.cur = 0; 
}


/* REMOVECHILD
 * In:  structure pointer to task list
 * Out: none
 * Descr: called by removeTask(), removes the most 
 *        recent PID from the task library
 */
void _removeChild( struct Child *foo ){


  	if( foo->val == foo->nxt->val ){
		// if c is the only link in the deque, set curr to null
	  	t.cur = 0;
	}
	else{
		foo->nxt->prv = foo->prv;
		foo->prv->nxt = foo->nxt;
		// if c is the last link, change curr to the previous of c
		if( foo->val == t.cur->val ){
			t.cur = t.cur->prv;
        }
	}
	
//printf("del val %d\n",foo->val);

	free(foo);
	t.siz--;
}


/* REMOVE TASK
 * In:  none
 * Out: none
 * Descr: removes ALL PID values from the task list library
 */
void removeTask() 
{

    struct c* cp;
    int status;

    waitpid(t.cur->val, &status, 0);

    cp = t.cur;
    if(t.siz > 1){
          t.cur = t.cur->prv;
    }
   _removeChild(cp);
}


/* GETCHILD
 * In:  none
 * Out: int to PID value
 * Descr: retrieves the most recent PID value added to 
 *        the task list library
 */
int getPidt() 
{

    if(t.siz > 1){
        t.cur = t.cur->nxt;
        return t.cur->prv->val;
    }
    else{
        return t.cur->val;
    }
}




// *********************************************************************
// ************************  SIGNALS  **********************************
// *********************************************************************



/* FNSIGSTOP
 * In:  none
 * Out: none
 * Descr: function for handling CTRL-Z-invoked stop signals; 
 *        toggles the enabling/disabling of background tasks
 */
static void fnsigstop(){

    int foodump;

    // notify user
	char* msg = "SIGTSTP invoked\n";
	write(STDOUT_FILENO, msg, 16);

    // switch background permission flag
    // (foodump is inert/throwaway)
    foodump=bgdAllowed(-1);

    // return to point of interruption
    return;
}


/* FNINTERRUPT
 * In:  none
 * Out: none
 * Descr: function for handling CTRL-C-invoked interrupt signals
 */
static void fninterrupt()
{  
    // notify user
	char* msg = "SIGINT caught\n";
	write(STDOUT_FILENO, msg, 18);

    // go to exit
    exit(0);
}


/* FNSIGCHLD
 * In:  none
 * Out: none
 * Descr: invoking several built-in signal functions, this
 *        function is checking for the status of the most
 *        recently added task by looking for a returned signal
 */
static void fnsigchld(){

    int stat;

    // the following code directly implements
    // Michael Kerrisk's signal handling

    /* supplemental resource:                 **
    **                                        **
    ** (these status return values, copied in **
    ** its entirety)                          **
    **                                        **
    ** Kerrisk, Michael. "Listing 26-2,       **
    ** procexec/print_wait_status.c." The     **
    ** Linux Programming Interface: A Linux   **
    ** and UNIX System Programming Handbook.  **
    ** San Francisco: No Starch Press, 2010.  **
    ** 546-547,                               */

    if(WIFEXITED(stat)){
        printf("Task exited, status: %d\n", 
        WEXITSTATUS(stat)); fflush(stdout);
    } 
    else if (WIFSIGNALED(stat)){
        printf("Task terminated, signal %d (%s)",
        WTERMSIG(stat), strsignal(WTERMSIG(stat)));
        fflush(stdout);
        #ifdef WCOREDUMP
        if (WCOREDUMP(stat)){
            printf(" (core dumped)");
        }
        #endif
        printf("\n");
    } 
    else if (WIFSTOPPED(stat)){
        printf("Task stopped, signal %d (%s)\n", 
        WSTOPSIG(stat), strsignal(WSTOPSIG(stat)));
        fflush(stdout);
        #ifdef WIFCONTINUED
    } 
    else if (WIFCONTINUED(stat)){
        printf("Task continued\n"); fflush(stdout);
        #endif
    }else{            
        
        fflush(stdout);
    }

    return;
}




// *********************************************************************
// **********************  TASK MNGMT  *********************************
// *********************************************************************



/* CHECKTASKDONE
 * In:  PID integer value
 * Out: none
 * Descr: invoking several built-in signal functions, this
 *        function is checking for the status of the most
 *        recently added task by looking for a returned signal
 */
void checkTaskDone(int p){

    int stat;

    waitpid(p, &stat, WNOHANG);


    // the following code directly implements
    // Michael Kerrisk's signal handling

    /* supplemental resource:                 **
    **                                        **
    ** (these status return values, copied in **
    ** its entirety)                          **
    **                                        **
    ** Kerrisk, Michael. "Listing 26-2,       **
    ** procexec/print_wait_status.c." The     **
    ** Linux Programming Interface: A Linux   **
    ** and UNIX System Programming Handbook.  **
    ** San Francisco: No Starch Press, 2010.  **
    ** 546-547,                               */

    if(WIFEXITED(stat)){
        printf("Task %d exited, status of %d\n", 
        p,WEXITSTATUS(stat));
    } 
    else if (WIFSIGNALED(stat)){
        printf("Task %d terminated, signal %d (%s)",
        p,WTERMSIG(stat), strsignal(WTERMSIG(stat)));
        #ifdef WCOREDUMP
        if (WCOREDUMP(stat)){
            printf(" (core dumped)");
        }
        #endif
        printf("\n"); fflush(stdout);
    } 
    else if (WIFSTOPPED(stat)){
        printf("Task %s stopped, signal %d (%s)\n", 
        p,WSTOPSIG(stat), strsignal(WSTOPSIG(stat)));
        fflush(stdout);
        #ifdef WIFCONTINUED
    } 
    else if (WIFCONTINUED(stat)){
        printf("Task continued\n"); fflush(stdout);
        #endif
    }else{            
        
        fflush(stdout);
    }
}


/* CHECKALLTASKSDONE
 * In:  none
 * Out: none
 * Descr: invokes checkTaskDone() for as many tasks 
 *        as are presently populating the task list 
 *        library; utilizes getPidt() to retrieve our
 *        child task ID value 
 */
void checkAllTasksDone(){

    int x;

    if(t.siz > 0){
        for(x=0; x < t.siz; x++){
            checkTaskDone(getPidt());
        }
    }
}



// *********************************************************************
// ***********************  BUILT-IN  **********************************
// *********************************************************************


/* BINCD
 * In:  char-ptr, location argument
 * Out: none
 * Descr: recreating same function as bash's "cd"
 * built-in, changes current directory; first, if
 * user didn't pass-in a specific argument, assigns
 * argument for current directory, "."; second, sets
 * the environment to system Home root; third, passes
 * directory argument to "chdir" to update current dir
 */
void bincd( char* arg ){

    char cwd[PATH_MAX];

    /* supplemental resource:                       **
    **                                              **
    ** Free Software Foundation, Inc. "Ftp Server." **
    ** ftpd.c. GNU Inetutils, 2015.                 */

    // if( arg == NULL ){ arg = "."; }
    //setenv ("HOME", arg, 1);
    if( arg == NULL ){ arg = getenv("HOME"); }

    // change directory
    if( chdir(arg) < 0 ){   
        perror("BIN CD error chdir\n"); return;
    }
    else{
        printf( "chdir exec to \"%s\"\n",arg );
        fflush(stdout);return;
    }
}


/* BINLS
 * In:  none
 * Out: none
 * Descr: recreating same function as bash's "ls" 
 * built-in, opens current directory; reads names
 * of directory's symbolically-linked contents, 
 * printing each result 
 */
void binls(int x, char* arg ){

    DIR* dr;
    struct dirent *rn;

    int ptask, status;

    if( x == -1 ){
        // open dir
        if((dr = opendir(".")) == NULL){
            printf("BIN LS error opendir \".\"\n\n"); exit(EXIT_FAILURE);
        }

        // print contents
        while((rn = readdir(dr)) != NULL){
            if(rn->d_name[0] == '.'){ continue; }
            if(rn->d_name[0] == "." && rn->d_name[1] == "."){ continue; }

            // printf("  - %s\n", rn->d_name);
            fprintf(stdout,"  - %s\n", rn->d_name);
        }
        closedir(dr);
        fflush(stdout);
    }
    else{
        ptask=fork();
        if( ptask == -1 ){
            // error catch
            perror("FORK error signaled\n");exit(1);
        }
        else if( ptask == 0 ){

            // open dir
            if((dr = opendir(".")) == NULL){
                printf("BIN LS error opendir \".\"\n\n"); exit(EXIT_FAILURE);
            }

            x=open(arg,O_WRONLY|O_CREAT|O_TRUNC,0644);

            dup2(x,STDOUT_FILENO);
            close(x);

            // print contents
            while((rn = readdir(dr)) != NULL){
                if(rn->d_name[0] == '.'){ continue; }
                if(rn->d_name[0] == "." && rn->d_name[1] == "."){ continue; }
                fprintf(stdout,"  - %s", rn->d_name);
            }
            fflush(stdout);
            fclose(arg);
            closedir(dr);
        }
        else{
            waitpid(ptask,&status,0);
        }
    }
}


/* BINSTATUS
 * In:  none
 * Out: none
 * Descr: calls "fnsigchld()" to retrieve most-
 * recent-returned signal (from child proc) 
 */
void binstatus(){
    fnsigchld();
}

/* REMOVEALLTASKS
 * In:  none
 * Out: none
 * Descr: until t.siz eq's zero, calls "removetask()"
 */
void removeAllTasks(){
	while( t.siz > 0 ){
		removeTask();		
    }
}



// *********************************************************************
// *********************************************************************
// *********************************************************************




/* HANDLEINPUT
 * In:  ptr to structure
 * Out: signaling integer
 * Descr: 
 */
int handleInput(INLN* ln)
{
    int u,w,x,y,z, backflag, count, fd, fdi, fdiflag, fdo, fdoflag, status;
    fd=fdi=fdo=fdiflag=fdoflag=-1;
    w=x=y=z=backflag=count=0;
    char ch;
    char line[20];
    char nptr[] = "NULL";
    pid_t ptask;

    // blankline handler
    if(ln->wds[0].word == NULL){
        return 0;
    }

    // (&) background Marker
    ch = ln->wds[ln->numWds-1].word[0];
    if(ch == 38){
        backflag = 1;
    }
    else{
        backflag = 0;
    }

    // (#) comment handler
    if( auditLine("#",ln->wds[0].word) == 1 ){
        return 0;
    }

    while( w < ln->maxWds ){
        // ($$) PID handler
        if( auditForPID("$$",ln->wds[w].word) == 1 ){
            pidHandler( ln, w );
        }
        w++;
    }

/**
    // (bin) LS
    if( auditLine("LS",ln->wds[0].word) == 1 ){
        w=0;
        x=-1;
        while( w < ln->maxWds ){
            if( auditLine(">",ln->wds[w].word) == 1 ){
                x=(w+1);
            }
            w++;
        }
        if( x == -1 ){
            binls(x,nptr);
        }
        else{
            binls(x,ln->wds[x].word);
        }
        return 0;
    }
**/

    // (bin) CD
    if( auditLine("CD",ln->wds[0].word) == 1 ){
        if(ln->wds[x+1].word == NULL || ln->wds[x+1].word == ""){
            // bincd(".");
            bincd(NULL);
        }
        else if(ln->numWds == 1){
            // bincd(".");
            bincd(NULL);
        }
        else{
            bincd(ln->wds[x+1].word);
        }
        return 0;
    }
    // (bin) EXIT, -1 quit program
    else if( auditLine("EXIT",ln->wds[0].word) == 1 ){ 
        removeAllTasks();
        return -1; 
    }
    // if CHECK 
    else if( auditLine("CHECK",ln->wds[0].word) == 1 ){
        checkAllTasksDone();
        return 0;
    }
    // if STATUS,
    else if( auditLine("STATUS",ln->wds[0].word) == 1 ){
        binstatus();
        return 0;
    }
    // otherwise, init loop CHILDPID --> FORK-REG + EXECLP()
    else{ 
        char** cmds = (char**)malloc(sizeof(char *) * ln->numWds);
        char* incmd = (char*)malloc(sizeof(char *) * 20);
        char* outcmd = (char*)malloc(sizeof(char *) * 20);
        for(x=0,y=0; x<ln->numWds;x++){
            // if STDIN redirect instr, set flag
            if(auditLine("<",ln->wds[x].word) == 1){
                // flag assigned num of the next argument in cmd-line
                fdiflag = x+1;

                u=0;
                while( ln->wds[x+1].word[u] ){
                    incmd[u] = ln->wds[x+1].word[u];
                    u++;
                }

                x++;
            }
            // if STDOUT redirect instr, set flag
            else if (auditLine(">",ln->wds[x].word) == 1){
                // flag assigned num of the next argument in cmd-line
                fdoflag = x+1;

                u=0;
                while( ln->wds[x+1].word[u] ){
                    outcmd[u] = ln->wds[x+1].word[u];
                    u++;
                }

                x++;
            }
            // protect against prohibited exec() characters
            else{
                if((auditLine("&",ln->wds[y].word) != 1)
                && (auditLine("!",ln->wds[y].word) != 1) 
                && (auditLine("|",ln->wds[y].word) != 1)){
                    cmds[y] = ln->wds[x].word;
                    y=y+1;
                }
            }
        }
        // insert '\0' signaler at end, 
        // per execvp() construction
        cmds[y] = NULL;


/**
        // if STDIN flag set...
        if(fdiflag != -1 ){
            // open file using cmd-ln argument definition
            fdi=open(incmd,O_RDONLY);
        }
        // else, if
        if(fdiflag == -1 && backflag == 1){
            // open directed to 'dev/null'
            fdi=open("/dev/null",O_RDONLY);
        }   

        // 
        dup2(fdi,0);
        close(fdi);


        // if STDOUT flag set...
        if(fdoflag != -1){
            fdo=open(ln->wds[fdoflag].word,
            O_WRONLY|O_CREAT|O_TRUNC,0644);
        }
        if(fdoflag == -1 && backflag == 1){
            // open directed to 'dev/null'
            fdo=open("/dev/null",O_WRONLY|O_CREAT,0744);
        }  
 
        // 
        dup2(fdo,STDOUT_FILENO);
        close(fdo);
**/ 



            /* supplemental resources - fd, execvp    **
            **                                        **
            ** Duarte, Carlos. "dumdo.c." CGD's C     **
            ** Grab Bag, Last modified June 19, 1997. **
            ** http://cgd.sdf-eu.org/a_cgb.html       **
            **                                        **
            ** Kerrisk, Michael. "Listing 27-3,       **
            ** procexec/t_execlp.c." The Linux Progr- **
            ** amming Interface: A Linux and UNIX     **
            ** System Programming Handbook. San Fran- **
            ** cisco: No Starch Press, 2010. 569,     **
            **                                        **
            ** "Listing 28-3, procexec/t_clone.c."    **
            ** The Linux Programming Interface. 601.  **
            **                                        **
            ** Von Leitner, Felix. "http.c." Gatling  **
            **  - A High Performance Web Server.      **
            ** https://www.fefe.de/gatling/           */

            // Fork!
            ptask=fork();
            if( ptask == -1 ){
                // error catch
                perror("FORK error signaled\n");exit(1);
            }
            else if( ptask == 0 ){
                // if STDIN flag set...
                if(fdiflag != -1 ){
                    // open file using cmd-ln argument definition
                    fdi=open(incmd,O_RDONLY);
                }
                // else, if
                if(fdiflag == -1 && backflag == 1){
                    // open directed to 'dev/null'
                    fdi=open("/dev/null",O_RDONLY);
                }   
                dup2(fdi,0);
                close(fdi);

                // if STDOUT flag set...
                if(fdoflag != -1){
                    fdo=open(outcmd,O_WRONLY|O_CREAT|O_TRUNC,0644);
                }
                if(fdoflag == -1 && backflag == 1){
                    // open directed to 'dev/null'
                    fdo=open("/dev/null",O_WRONLY|O_CREAT,0744);
                }  
                dup2(fdo,STDOUT_FILENO);
                close(fdo);

                /* supplemental resources - execvp        **
                **                                        **
                ** Kerrisk, Michael. "27.2.1 The PATH     **
                ** Environment Variable" The Linux Progr- **
                ** amming Interface: A Linux and UNIX     **
                ** System Programming Handbook. San Fran- **
                ** cisco: No Starch Press, 2010. 568,     */

                execvp(cmds[0],cmds);

                // if fail...
                // perror("EXECVP failure\n");
                // free arguments
                free(cmds);
                free(incmd);
                free(outcmd);
                // exit: fail
                exit(1); 
            }
            else{

                if( (backflag == 1) && (bgdAllowed(1) == 1) ){

                    printf("Bkgd task added %d)\n",ptask);fflush(stdout);
                    addTask(ptask);
                }
                else if( (backflag == 1) && (bgdAllowed(1) == -1) ){
                    printf("(background execution not permitted)\n");
                            fflush(stdout);
                    waitpid(ptask,&status,0);
                }
                else{

    // WARNING, STDOUT GOES RIGHT INTO A NEW FILE IF MID-OP BECAUSE OF PARALLEL OPS

                    // printf("(running %d)\n",ptask);fflush(stdout);
                    waitpid(ptask,&status,0);
                }

                return 0; 
            }
        
    }
}



/* DOLOOP
 * In:  ptr to structure
 * Out: none
 * Descr: responsible for printing the terminal prompt,
 *        retrieving user-terminal input, and handling 
 *        (i.e., executing) user instruction
 */
void doLoop(INLN* ln)
{
    char c;  int v,x,y,z; 
    int numCharsEntered;
	size_t bufferSize = 0;
    char* lineEntered = NULL;

    ln->numWds = 0;
    ln->maxWds = 512;
    v=0;


    while(v!=-1){
        /** GET USER INPUT **/
        // make input space
        ln->wds = (INWD*)malloc(ln->maxWds * sizeof(INWD));
        for(y=0; y<ln->maxWds; y++){
            for(x=0; x<2048; x++){
                ln->wds[y].word[x] = '\0';
            }
        }
        // get input from user
        while(1){
            // Print Prompt
            printf(": ");
            numCharsEntered = getline(&lineEntered, &bufferSize, stdin); 
            if(numCharsEntered == -1){
                clearerr(stdin);
            }
            else{
                break; // Exit the loop - we've got input
            }
        }
        // parse input from user
        x=y=z=0;
        while(lineEntered[z] != '\n'){  // '\n'
            if(lineEntered[z] == 32){    // ' '
                ln->wds[x].word[y] = 0;  // '\0'
                y = 0;  // set word ltr to 0
                x++;    // incr next word
                z++;    // incr next line ltr
            }
            else{       // else copy letter
                ln->wds[x].word[y] = lineEntered[z];
                y++;
                z++;
            }
        }
        ln->numWds = x+1;

        // handle user's input command
        v=handleInput(ln);

        // check for completed tasks before returning prompt to user
        checkAllTasksDone();

        free(ln->wds);
        free(lineEntered);
        lineEntered=NULL;
    }
}



/* MAIN
 * In:  none
 * Out: status int, signalling successful operation
 */
int main( ){
    struct sigaction sgp;
    struct sigaction sgi;
   // struct sigaction sgc;
    sgp.sa_handler = fnsigstop;
    sgi.sa_handler = fninterrupt;
    //sgc.sa_handler = fnsigchld;
    sgp.sa_flags = 0;
    sgi.sa_flags = 0;
    //sgc.sa_flags = 0;
    sigfillset(&(sgp.sa_mask));
    sigaction(SIGTSTP, &sgp, NULL);  // sgs --> SIGSTOP, ctrl+z
    sigfillset(&(sgi.sa_mask));
    sigaction(SIGINT, &sgi, NULL);   // sgi --> sigint, ctrl+c
    //sigfillset(&(sgc.sa_mask));
    //sigaction(SIGCHLD, &sgc, NULL);  // sgc --> SIGCHLD, ended proc

    struct INLN ln;
 
   /* init children */
    initTaskList();
    doLoop(&ln);
    removeAllTasks();
    return 0;
}