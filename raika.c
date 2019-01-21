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
pthread_mutex_t M_LOCK;

struct STRUCT_WIN_LOOP 
{
    //universal window variables
    int I_SCREEN;
    Window W_WIN;

    //child windows (menu buttons)
    int **IP_X;
    int **IP_Y;
    int **IP_WIDTH;
    int **IP_HEIGHT;
  
    unsigned int **IU_BORDER_WIDTH;
    unsigned int **IU_DEPTH;
    
    Window **PW_CHILD_WIN;
    
    int j;
};

void *CHILD_WINDOW(void *ARG)  //type and passing type requirement for use in thread
{
        struct STRUCT_WIN_LOOP *S_WIN_LOOP = ARG;
    
        int l = S_WIN_LOOP->j;
        printf("%d\n", l);

        BREAK = 1;
        //printf("you've made it this far %d\t%d\t%d\n", l, *S_WIN_LOOP->PW_CHILD_WIN[l], EVENT.xany.window);
        
        XEvent LOCAL_EVENT;
        
    while(1)
    {
        printf("Loop Thread %d\n",l);
        
        XWindowEvent(DP_DPY, *S_WIN_LOOP->PW_CHILD_WIN[l], ExposureMask | KeyPressMask | EnterWindowMask | LeaveWindowMask, &LOCAL_EVENT);
        
            if(LOCAL_EVENT.xany.window == *S_WIN_LOOP->PW_CHILD_WIN[l])
            {
                
                if (LOCAL_EVENT.type == LeaveNotify)
                {
                    XClearWindow(DP_DPY, *S_WIN_LOOP->PW_CHILD_WIN[l]);
                }
            
                if (LOCAL_EVENT.type == EnterNotify)
                {
                   XDrawLine(DP_DPY, *S_WIN_LOOP->PW_CHILD_WIN[l], DefaultGC(DP_DPY, S_WIN_LOOP->I_SCREEN), 20, 10, 60, 60);
                }
                
                if(LOCAL_EVENT.type == Expose)
                {
                    //XDrawLine(DP_DPY, *S_WIN_LOOP->PW_CHILD_WIN[l], DefaultGC(DP_DPY, S_WIN_LOOP->I_SCREEN), 20, 10, 60, 60);
                }
            }
    }
}

int main()
{
    XInitThreads();
    
    XEvent EVENT;   //im assuming that this changes a lot, ie every time theres an event. so all the threaded functions needs this global

    
    //geeksforgeeks.org/mutex-lock-for-linux-thread-synchronization , for error handling -- has good thrad error handling too
    //pthread_mutex_init(&M_LOCK, NULL);     
    //no longer needed
    
    int k;

    struct STRUCT_WIN_LOOP S_WIN_LOOP;
    
    DP_DPY=XOpenDisplay(NULL);
    
    if (DP_DPY == NULL)
    {
        exit (1);
    }

    S_WIN_LOOP.I_SCREEN=DefaultScreen(DP_DPY);
    S_WIN_LOOP.W_WIN=XCreateSimpleWindow(DP_DPY,RootWindow(DP_DPY, S_WIN_LOOP.I_SCREEN), 200,200,500,500,1, BlackPixel(DP_DPY, S_WIN_LOOP.I_SCREEN), WhitePixel(DP_DPY, S_WIN_LOOP.I_SCREEN));
    
    XSelectInput(DP_DPY, S_WIN_LOOP.W_WIN, ExposureMask);
    XMapWindow(DP_DPY, S_WIN_LOOP.W_WIN);
    
    //not really sure what this i does
    Window W_ROOT_WIN;
    
    //how many loops
    int i=9;
    int SPACE=105;
    int Y_POS=20;
    
    
    //initializing loop variables    
    S_WIN_LOOP.PW_CHILD_WIN=malloc( ((sizeof(S_WIN_LOOP.PW_CHILD_WIN)*i)) );//size times the number of pointers
    S_WIN_LOOP.IP_X=malloc( ((sizeof(S_WIN_LOOP.IP_X)*i)) );
    S_WIN_LOOP.IP_Y=malloc( ((sizeof(S_WIN_LOOP.IP_Y)*i)) );
    S_WIN_LOOP.IP_WIDTH=malloc( ((sizeof(S_WIN_LOOP.IP_WIDTH)*i)) );
    
    S_WIN_LOOP.IP_HEIGHT=malloc( ((sizeof(S_WIN_LOOP.IP_HEIGHT)*i)) );
    S_WIN_LOOP.IU_BORDER_WIDTH=malloc( ((sizeof(S_WIN_LOOP.IU_BORDER_WIDTH)*i)) );
    S_WIN_LOOP.IU_DEPTH=malloc( ((sizeof(S_WIN_LOOP.IU_DEPTH)*i)) );
    
    pthread_t **P_THREAD;
    P_THREAD=malloc( ((sizeof(P_THREAD) * i )) );

    
    for (S_WIN_LOOP.j=0; S_WIN_LOOP.j < i; S_WIN_LOOP.j++)
    {
        BREAK = 0;
        k=S_WIN_LOOP.j; //purely for readablity
        S_WIN_LOOP.PW_CHILD_WIN[k]=malloc(sizeof(Window)); //size of the window thing
        S_WIN_LOOP.IP_X[k]=malloc(sizeof(int)); 
        S_WIN_LOOP.IP_Y[k]=malloc(sizeof(int));
        S_WIN_LOOP.IP_WIDTH[k]=malloc(sizeof(int));
        S_WIN_LOOP.IP_HEIGHT[k]=malloc(sizeof(int));
        S_WIN_LOOP.IU_BORDER_WIDTH[k]=malloc(sizeof(unsigned int));
        S_WIN_LOOP.IU_DEPTH[k]=malloc(sizeof(unsigned int));
        
        *S_WIN_LOOP.PW_CHILD_WIN[k]=XCreateSimpleWindow(DP_DPY, S_WIN_LOOP.W_WIN, 20,Y_POS,200,100,1, BlackPixel(DP_DPY, S_WIN_LOOP.I_SCREEN), WhitePixel(DP_DPY, S_WIN_LOOP.I_SCREEN));
        
        XSelectInput(DP_DPY, *S_WIN_LOOP.PW_CHILD_WIN[k], ExposureMask | KeyPressMask | EnterWindowMask | LeaveWindowMask);
        XMapWindow(DP_DPY, *S_WIN_LOOP.PW_CHILD_WIN[k]);
        //XGetGeometry(DP_DPY, *S_WIN_LOOP.PW_CHILD_WIN[k], &W_ROOT_WIN, &S_WIN_LOOP.IP_X[k], &S_WIN_LOOP.IP_Y[k], &S_WIN_LOOP.IP_WIDTH[k], &S_WIN_LOOP.IP_HEIGHT[k], &S_WIN_LOOP.IU_BORDER_WIDTH[k], &S_WIN_LOOP.IU_DEPTH[k]);
        
        Y_POS=(( Y_POS + SPACE ));
        
        
        P_THREAD[k]=malloc(sizeof(pthread_t));
        pthread_create(P_THREAD[k], NULL, CHILD_WINDOW, &S_WIN_LOOP);
        while (BREAK != 1);
            
    }
    
    

        
    printf("%d\n", S_WIN_LOOP.j);
    while (1)
    {
        sleep(100);
    };
    
    return 0;
}
