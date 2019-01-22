#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h> 
#include <X11/Xatom.h>
#include <pthread.h>
#include <unistd.h>

pthread_mutex_t LOCK;

Display *DP_DPY;    //look into what xorg does with this

Window W_WIN;

Window W_ROOT_WIN;

pthread_t **P_THREAD;

int I_Thread_Ctr=0;

int I_Total_Menu_Item=50; // arbitararily high-- for now.

int I_SCREEN;

                    //XResizeWindow(DP_DPY, W_WIN, 800, 800); Use this for expanding Menus (their own thread type)
                    //XMoveWindow if its too close to an edge
                    //XGetGeometry for the window dimensions / location
                    //make struct for user defined variables, and window size/location

                    //child windows spawn nothing. will always be unique.
                    //give them direct access to their parent matrix
                    //Example[parent][self]

struct Struct_Win_Char
{
    int I_MenuX;
    int I_MenuY;
    int I_MenuWidth;
    int I_MenuHeight;
    int I_MenuThreads;
    int I_WindowCtr;
    int *I_ParentWin;
    int *I_SelfWin;
    
    Window *LOCAL_W_PTR;
};

void *CHILD_WINDOW_L (void *);
void *CHILD_WINDOW_S (void *);
void *SUB_LAUNCH_MENU (void *);
void *ROOT_LAUNCH_MENU (void *);

void *WINDOW_DISPLAY(void *Pass)    //clean this naming nightmare up later
{
    struct Struct_Win_Char *Local = Pass;

    while (1)
    {

                if(*Local->I_ParentWin == 1)
                {
                    XMapWindow(DP_DPY, *Local->LOCAL_W_PTR);
                    printf("happening");
                }
                else
                {
                    XUnmapWindow(DP_DPY, *Local->LOCAL_W_PTR);
                    printf("not happening");

                }
    
    }
}

/*
int COMPARE(int Arg, int ArgA[], int ArgH )
{
    for (int i=0; i < ArgH; i++)
    {
        if (ArgA[i] == Arg)
        {
            return (1);
        }
    }
    return (0);
}
*/

void *CHILD_WINDOW_L(void *Pass)    
{
        //making a deep copy
        struct Struct_Win_Char *Temp = Pass;
        struct Struct_Win_Char S_Loc_Char;
        
        S_Loc_Char.I_MenuX=Temp->I_MenuX;
        S_Loc_Char.I_MenuY=Temp->I_MenuY;
        S_Loc_Char.I_MenuWidth=Temp->I_MenuWidth;
        S_Loc_Char.I_MenuHeight=Temp->I_MenuHeight;
        S_Loc_Char.I_WindowCtr=Temp->I_WindowCtr;
        S_Loc_Char.I_ParentWin=Temp->I_SelfWin;           //this is correct!

        
        
        XEvent LOCAL_EVENT;
        Window W_CHILD_WIN;
        S_Loc_Char.LOCAL_W_PTR=&W_CHILD_WIN;
        
        int I_X;
        int I_Y;
        int I_WIDTH;
        int I_HEIGHT;
        unsigned int IU_BORDER_WIDTH;
        unsigned int IU_DEPTH;
        
        W_CHILD_WIN=XCreateSimpleWindow(DP_DPY, W_WIN, S_Loc_Char.I_MenuX, S_Loc_Char.I_MenuY, S_Loc_Char.I_MenuWidth, S_Loc_Char.I_MenuHeight, 1, BlackPixel(DP_DPY, I_SCREEN), WhitePixel(DP_DPY, I_SCREEN));
        
        XSelectInput(DP_DPY, W_CHILD_WIN, ExposureMask | EnterWindowMask | LeaveWindowMask);
        
       if (*S_Loc_Char.I_ParentWin != 1)    //THIS IS FUCKING SLOW AS FUCK. come up with a better solution
        {  
            int k=I_Thread_Ctr;
            P_THREAD[k]=malloc(sizeof(pthread_t));
            pthread_create(P_THREAD[k], NULL, WINDOW_DISPLAY, &S_Loc_Char); 
            I_Thread_Ctr++;
        }
        else   
        {
            XMapWindow(DP_DPY, W_CHILD_WIN);
        }

    XMapWindow(DP_DPY, W_CHILD_WIN);
    XGetGeometry(DP_DPY, W_CHILD_WIN, &W_ROOT_WIN, &I_X, &I_Y, &I_WIDTH, &I_HEIGHT, &IU_BORDER_WIDTH, &IU_DEPTH);   //this is an asynchronous function, and (sometimes) it is waiting for information that is somehow connected to the queue.
            
        pthread_mutex_lock(&LOCK);
        printf("Locked and unlocked!\n");
        pthread_mutex_unlock(&LOCK);
        
    while(1)
    {        
        
        XWindowEvent(DP_DPY, W_CHILD_WIN, ExposureMask | EnterWindowMask | LeaveWindowMask, &LOCAL_EVENT);

            if(LOCAL_EVENT.xany.window == W_CHILD_WIN)
            {
                            
                if (LOCAL_EVENT.type == EnterNotify)
                {
                    *S_Loc_Char.I_ParentWin=1;

                    XDrawLine(DP_DPY, W_CHILD_WIN, DefaultGC(DP_DPY, I_SCREEN), 20, 10, 60, 60);
                    printf("Enter Window %d\n", S_Loc_Char.I_WindowCtr);
                }
                    
                if (LOCAL_EVENT.type == LeaveNotify)
                {

                    *S_Loc_Char.I_ParentWin=0;
                    
                    XClearWindow(DP_DPY, W_CHILD_WIN);
                    printf("Leave Window %d\n",S_Loc_Char.I_WindowCtr);
                }
                
                if(LOCAL_EVENT.type == Expose)
                {
                    printf("Window in view! %d\n",S_Loc_Char.I_WindowCtr);
                }
                
            }
            
    }
}

void *CHILD_WINDOW_S(void *Pass)    //under development
{
        //making a deep copy
        struct Struct_Win_Char *Temp = Pass;
        struct Struct_Win_Char S_Loc_Char;
        
        S_Loc_Char.I_MenuX=Temp->I_MenuX;
        S_Loc_Char.I_MenuY=Temp->I_MenuY;
        S_Loc_Char.I_MenuWidth=Temp->I_MenuWidth;
        S_Loc_Char.I_MenuHeight=Temp->I_MenuHeight;
        S_Loc_Char.I_WindowCtr=Temp->I_WindowCtr;

        //inheritance varables
        int I_Master=0;
        S_Loc_Char.I_ParentWin=Temp->I_ParentWin;           //this is correct!
        S_Loc_Char.I_SelfWin=&I_Master;       
                
        XEvent LOCAL_EVENT;
        Window W_CHILD_WIN;
        int I_X;
        int I_Y;
        int I_WIDTH;
        int I_HEIGHT;
        unsigned int IU_BORDER_WIDTH;
        unsigned int IU_DEPTH;
        
        W_CHILD_WIN=XCreateSimpleWindow(DP_DPY, W_WIN, S_Loc_Char.I_MenuX, S_Loc_Char.I_MenuY, S_Loc_Char.I_MenuWidth, S_Loc_Char.I_MenuHeight, 1, BlackPixel(DP_DPY, I_SCREEN), WhitePixel(DP_DPY, I_SCREEN));
        XSelectInput(DP_DPY, W_CHILD_WIN, ExposureMask | EnterWindowMask | LeaveWindowMask);
        XMapWindow(DP_DPY, W_CHILD_WIN);
        
        S_Loc_Char.I_MenuX=(( Temp->I_MenuX + Temp->I_MenuWidth ));
        SUB_LAUNCH_MENU( &S_Loc_Char );
        
        pthread_mutex_lock(&LOCK);
        printf("Locked and unlocked!\n");
        pthread_mutex_unlock(&LOCK);

    while(1)
    {        
        
        XWindowEvent(DP_DPY, W_CHILD_WIN, ExposureMask | EnterWindowMask | LeaveWindowMask, &LOCAL_EVENT);
        
            if(LOCAL_EVENT.xany.window == W_CHILD_WIN)
            {
                
                if (LOCAL_EVENT.type == EnterNotify)
                {
                    *S_Loc_Char.I_SelfWin=1;
                    
                    XDrawLine(DP_DPY, W_CHILD_WIN, DefaultGC(DP_DPY, I_SCREEN), 20, 10, 60, 60);
                    printf("Enter Window %d\tSelfWin %d\n",S_Loc_Char.I_WindowCtr, *S_Loc_Char.I_SelfWin);
                }
                
                if (LOCAL_EVENT.type == LeaveNotify)
                {

                    *S_Loc_Char.I_SelfWin=0;
                        
                    XClearWindow(DP_DPY, W_CHILD_WIN);
                    printf("Leave Window %d\tSelfWin %d\n",S_Loc_Char.I_WindowCtr, *S_Loc_Char.I_SelfWin);
                }
            
                if(LOCAL_EVENT.type == Expose)
                {
                    printf("Window in view! %d\n",S_Loc_Char.I_WindowCtr);
                }
            }
    }
}

void *ROOT_LAUNCH_MENU(void *Pass)
{
    struct Struct_Win_Char *S_Loc_Char = Pass;      //switch this over to use pass
        //struct Struct_Win_Char S_Loc_Char;
        
    int k;
    int i;
    int j;

    i=S_Loc_Char->I_MenuThreads;            //a function, will set this
        
    int EXAMPLE[]={0,1,0,0,0,0,0,0}; //this will be something made by the function that goes through menu file configs

    for (j=0; j < i; j++)
    {
        
        
        S_Loc_Char->I_WindowCtr=j;
        
        k=I_Thread_Ctr;
    
        P_THREAD[k]=malloc(sizeof(pthread_t));
        
        if (EXAMPLE[j] == 0)
        {
            pthread_create(P_THREAD[k], NULL, CHILD_WINDOW_L, S_Loc_Char);  //make an if statement here, spawn regular menu or sidebranch
        }
        else
        {
            pthread_create(P_THREAD[k], NULL, CHILD_WINDOW_S, S_Loc_Char); 

        }
        
        sleep (1);
        
        I_Thread_Ctr++;
                        
        S_Loc_Char->I_MenuY=(( S_Loc_Char->I_MenuY + S_Loc_Char->I_MenuHeight ));
         
    }
        
}
        
void *SUB_LAUNCH_MENU(void *Pass)               //temporary
{
        struct Struct_Win_Char *S_Loc_Char = Pass;      //switch this over to use pass
        //struct Struct_Win_Char S_Loc_Char;
        
        int k;
        int i=3;
        int j;
       /* 
        S_Loc_Char.I_MenuX=Temp->I_MenuX;
        S_Loc_Char.I_MenuY=Temp->I_MenuY;
        S_Loc_Char.I_MenuWidth=Temp->I_MenuWidth;
        S_Loc_Char.I_MenuHeight=Temp->I_MenuHeight;
        S_Loc_Char.I_MenuThreads=8;     //determined from as yet created function (same as below)
        S_Loc_Char.I_WindowCtr=Temp->I_WindowCtr;
        S_Loc_Char.I_SelfWin=Temp->I_SelfWin;
        */
               
        int EXAMPLE[]={0,0,0,0,0,0,0,0}; //this will be something made by the function that goes through menu file configs
    
    for (j=0; j < i; j++)
    {
       
        S_Loc_Char->I_WindowCtr=j;

        k=I_Thread_Ctr;
        
        P_THREAD[k]=malloc(sizeof(pthread_t));
        
        if (EXAMPLE[j] == 0)
        {
            pthread_create(P_THREAD[k], NULL, CHILD_WINDOW_L, S_Loc_Char);  //make an if statement here, spawn regular menu or sidebranch
        }
        else
        {
            pthread_create(P_THREAD[k], NULL, CHILD_WINDOW_S, S_Loc_Char); 

        }
        
        sleep (1);
        
        I_Thread_Ctr++;
                        
        S_Loc_Char->I_MenuY=(( S_Loc_Char->I_MenuY + S_Loc_Char->I_MenuHeight));
        
    }

}

int main()
{
    pthread_mutex_init(&LOCK, NULL);
    XInitThreads();
    XEvent EVENT; 
    
    struct Struct_Win_Char S_Win_Char;
   
    int i;
    int j;
    int SPACE;
    
    DP_DPY=XOpenDisplay(NULL);
    
    if (DP_DPY == NULL)
    {
        exit (1);
    }
    
    I_SCREEN=DefaultScreen(DP_DPY);
    W_WIN=XCreateSimpleWindow(DP_DPY,RootWindow(DP_DPY, I_SCREEN), 200,200,500,500,1, BlackPixel(DP_DPY, I_SCREEN), WhitePixel(DP_DPY, I_SCREEN));
    
    XSelectInput(DP_DPY, W_WIN, ExposureMask | FocusChangeMask);
    XMapWindow(DP_DPY, W_WIN);

    i=15;   //how many main menu windows
   
    int I_RootMaster=1;
    //setting root menu characterstics
    S_Win_Char.I_MenuWidth=200;
    S_Win_Char.I_MenuHeight=50;    
    S_Win_Char.I_MenuX=20; 
    S_Win_Char.I_MenuY=20; 
    S_Win_Char.I_WindowCtr=0;
    S_Win_Char.I_MenuThreads=8;         // determined by not yet made function. this one is the local # of stuff
    
    //taking care of master/slave relationship
    S_Win_Char.I_SelfWin=&I_RootMaster;         //im the main menu! you want to see me.
    S_Win_Char.I_ParentWin=&I_RootMaster;

    P_THREAD=malloc( ((sizeof(P_THREAD) * I_Total_Menu_Item )) );    
    
    pthread_mutex_lock(&LOCK);
    ROOT_LAUNCH_MENU( &S_Win_Char );
    pthread_mutex_unlock(&LOCK);

    while (1)
    {
        XWindowEvent(DP_DPY, W_WIN, ExposureMask | FocusChangeMask, &EVENT);
        if (EVENT.xany.window == W_WIN)
        {
            if (EVENT.type == FocusIn)
            {
                printf("Parent window in focus\n");
            }
            
            if (EVENT.type == FocusOut)
            {
                printf("Parent window out of focus\n");
            }
        }
    };
    
    return 0;
}


/*
 * 

pointer <- pointer <- pointer



All Window_L have a pointer to inherited parent value
All Window_S have a pointer to inherited parent value
All Window_S have a self value thats becomes the childs parent value

1
2 - 1
3   2
4   3
5
6
7 - 1
8   2 - 1
9   3   2 - 1
10  4   3   2
    5
    6 - 1
        2
        
if child pointer value = 1
    then parent poitner value = 1
    
    Window_L
    am i exposed? if parent value is 1 than im exposed
    parent value = 1 if im selected
    
    Window_S
    am i exposed? if parent value is 1 than im exposed
    parent value = 1 if im selected
    if self value = 1 than parent value = 1
        
*/








  //reads files, shortest to longest
        //file read funtion occurs in the respective alternate menu thread
        //each alternate menu thread inherits the previous counter, so it can keep track of what level its at, and what filename to build the menu off
    
        //starts with 0.men
        //MenArray[];
        //for positions that equal 0, launch regular menu
        //for positions that equal 1, raise flag - launch alternate menu (for example, on position 5)
    
        //in alternate menu
        //read 0-5.men
        //for positions that equal 0, launch regular menu
        //for poisitons that equal 1, raise flag - launch alternate menu (for example, on position 2)
    
        //in alternate menu
        //read 0-5-2.men
        //for positions that equal 0, launch regular menu
        //for poisitons that equal 1, raise flag - launch alternate menu (for example, on position 2)
    
        // user will provide total number of windows
    


