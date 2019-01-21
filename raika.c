#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h> 
#include <X11/Xatom.h>
#include <pthread.h>
#include <unistd.h>

Display *DP_DPY;    //look into what xorg does with this

int BREAK=0;        //come up with a better way to make sure threads are fully operatonal before incrementing loop. or dont?

Window W_WIN;

Window W_ROOT_WIN;

int I_SCREEN;

int Y_POS;

                    //XResizeWindow(DP_DPY, W_WIN, 800, 800); Use this for expanding Menus (their own thread type)
                    //XMoveWindow if its too close to an edge
                    //XGetGeometry for the window dimensions / location
                    //make struct for user defined variables, and window size/location

void *CHILD_WINDOW()
{
         printf("Thread Started\n");
 
        //printf("you've made it this far %d\t%d\t%d\n", l, *S_WIN_LOOP->PW_CHILD_WIN[l], EVENT.xany.window);
        
        XEvent LOCAL_EVENT;
        Window W_CHILD_WIN;
        int I_X;
        int I_Y;
        int I_WIDTH;
        int I_HEIGHT;
        unsigned int IU_BORDER_WIDTH;
        unsigned int IU_DEPTH;
        
        W_CHILD_WIN=XCreateSimpleWindow(DP_DPY, W_WIN, 20,Y_POS,200,50,1, BlackPixel(DP_DPY, I_SCREEN), WhitePixel(DP_DPY, I_SCREEN));
        
        XSelectInput(DP_DPY, W_CHILD_WIN, ExposureMask | EnterWindowMask | LeaveWindowMask);
        XMapWindow(DP_DPY, W_CHILD_WIN);
                
        BREAK = 1;
        XSync(DP_DPY, 1);
                
        XGetGeometry(DP_DPY, W_CHILD_WIN, &W_ROOT_WIN, &I_X, &I_Y, &I_WIDTH, &I_HEIGHT, &IU_BORDER_WIDTH, &IU_DEPTH);   //this is an asynchronous function, and (sometimes) it is waiting for information that is somehow connected to the queue.
                        
    while(1)
    {        
        
        XWindowEvent(DP_DPY, W_CHILD_WIN, ExposureMask | EnterWindowMask | LeaveWindowMask, &LOCAL_EVENT);
        
            if(LOCAL_EVENT.xany.window == W_CHILD_WIN)
            {
                
                if (LOCAL_EVENT.type == LeaveNotify)
                {
                    XClearWindow(DP_DPY, W_CHILD_WIN);
                    printf("Leave Window\n");

                }
            
                if (LOCAL_EVENT.type == EnterNotify)
                {
                    XDrawLine(DP_DPY, W_CHILD_WIN, DefaultGC(DP_DPY, I_SCREEN), 20, 10, 60, 60);
                    printf("Enter Window\n");

                }
                
                if(LOCAL_EVENT.type == Expose)
                {
                    printf("Window in view!\n");
                }
            }
    }
}

int main()
{
    XInitThreads();
    XEvent EVENT; 
    
    int j;
    int i;
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

    
    //not really sure what this i does
    //Window W_ROOT_WIN;
    // make these a struct in the future
    i=15;       //how many main menu items
    SPACE=55;   // this will become the same size as MenuYDim
                // MenuXDim
    Y_POS=20;   //global    
                //X_POS global
    
    pthread_t **P_THREAD;
    P_THREAD=malloc( ((sizeof(P_THREAD) * i )) );
    
    
    // make this a function-- MainMenuLoop
    for (j=0; j < i; j++)
    {
        BREAK = 0;
        
        P_THREAD[j]=malloc(sizeof(pthread_t));
        pthread_create(P_THREAD[j], NULL, CHILD_WINDOW, NULL);  //make an if statement here, spawn regular menu or sidebranch
        
        while (BREAK != 1);
        Y_POS=(( Y_POS + SPACE ));
    }
    
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
