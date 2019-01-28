#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h> 
#include <X11/Xatom.h>

#include <pthread.h>

pthread_mutex_t G_Lock;                  //mutex global var

char *CPP_RaikaFileDir;        // the directory where menu item config files are

struct S_FileTracking
{
    
    int I_TotalWindowGroups;
    int I_WindowGroupCtr;
    int I_FileTypeLen;

    int *IP_FileDepthCounter;
    int *IP_FileTackCounter;
    
    char *CP_FileType;
    
    char **CPP_FileName;
    char **CPP_FileNameMatch;
    char **CPP_FileNameStripped;
    
    int TEMPCTR;
    
	DIR *D_Directory;
    struct dirent *entry;

};

struct S_WinChar
{
    
    int I_Xpos;
    int I_Ypos;     //this needs to be an array (IP)
    int I_Width;
    int I_Height;
    int I_ParentGroup;
    int I_WindowGroup;
    int I_RetrogradeWindowPos;
    
};

struct S_FileContents
{
    
    int I_FileTotalLines;
    int *IP_FileLineItemType;        
    char **CPP_FileLineItemName;
    char **CPP_FileLineItemAction;

    
};

void WindowConfigSet(void *Pointer);
void ConfigDirCheck(void *Pointer);
void FileNameAgg(void *Pointer);
void FileNameCompare(void *Pointer);
void FileDepthCounter(void *Pointer);
void FileDepthSorter (void *Pointer);
void FileNameStripper (void *Pointer);
void LaunchRoutine(void *PointerOne, void *PointerTwo);
void FileParser(char *DynamicFileOrder, void *Pointer);
void SetWindowSize(void *Pointer);
void XYCoordinates(char *DynamicFileOrderStripped, void *Pointer, int c);
void MenuItemThread(void *Pointer);
//void SubMenuThread(void *Pointer);

int main()
{

    //XInitThreads();                      //xorg thread support turned on
    //pthread_mutex_init(&G_Lock, NULL);   //initializing mutex
    struct S_FileTracking FTinit;
    FTinit.I_WindowGroupCtr=0;
      
    WindowConfigSet(&FTinit);
    ConfigDirCheck(&FTinit);
    
    FileNameAgg(&FTinit);
    FileNameCompare(&FTinit);
    
    FileDepthCounter(&FTinit);
    FileDepthSorter(&FTinit);
    
    FileNameStripper(&FTinit);
    
    struct S_WinChar WCinit[FTinit.I_TotalWindowGroups];
    
    LaunchRoutine(&FTinit, &WCinit);    //im a very, beefy function
    //Here will be where mapping is done

    return (0);
}

void WindowConfigSet(void *Pointer) {

    struct S_FileTracking *FTinit = Pointer;

    //setting the configuration direcotry
    
	//char *ACP_RaikaPath={"/.config/raika/"};
    char *ACP_RaikaPath={"/code/"};
    int I_RaikaBuff = strlen(ACP_RaikaPath);

	char *ACP_HomePath;
	ACP_HomePath=getenv("HOME");
    int I_HomeBuff = strlen(ACP_HomePath);

    CPP_RaikaFileDir=malloc(I_RaikaBuff + I_HomeBuff + 1);

    sprintf(CPP_RaikaFileDir, "%s%s", ACP_HomePath, ACP_RaikaPath);
   
}    
    
//checking if the direcotry is valid, and how many things are in the direcotry
//bring in D_Directory, and CPP_FileNameMatch, and CPP_FileName, and FileNAmebuff?
void ConfigDirCheck(void *Pointer) {

    struct S_FileTracking *FTinit = Pointer;
        
    FTinit->D_Directory = opendir(CPP_RaikaFileDir);
   
    if (FTinit->D_Directory != NULL)
	{
		closedir(FTinit->D_Directory);
	}

	else

	{
		exit(1);
	}

	opendir(CPP_RaikaFileDir);                  //add error handling? (same as above?)

    int h=0;
	while ((FTinit->entry = readdir(FTinit->D_Directory)) != NULL)
	{
		if (strcmp(FTinit->entry->d_name, ".") != 0 && strcmp(FTinit->entry->d_name, "..") != 0)
		{
			h++;
		}
	}

	rewinddir(FTinit->D_Directory);            //add error handling?
   
    
    
    FTinit->CPP_FileName=malloc(sizeof(void*) * h);
    FTinit->CPP_FileNameMatch=malloc(sizeof(void*) * h);
    //FTinit->I_FileNameLen=malloc(sizeof(int) * h);
}
    
    //bring in the opendir dir, the FTinit->D_Directory, ACP_FileName, I_FileNameLen
void FileNameAgg(void *Pointer) {
    //collects all files in a directory, ignoring current and parent directory
    struct S_FileTracking *FTinit = Pointer;
    
    int i=0;
	opendir(CPP_RaikaFileDir);             //add error handling
	while ((FTinit->entry = readdir(FTinit->D_Directory)) != NULL)
	{
		if (strcmp(FTinit->entry->d_name, ".") != 0 && strcmp(FTinit->entry->d_name, "..") != 0)
		{
			FTinit->CPP_FileName[i] = malloc(sizeof(FTinit->entry->d_name));
			strcpy(FTinit->CPP_FileName[i], FTinit->entry->d_name);
			i++;
		}
	}
    
    FTinit->TEMPCTR=i;          //STOPGAP UNTIL YOU REDO ALL YOUR LOOP VARS
    
	closedir(FTinit->D_Directory);                         //add error handling?

}
    
    //bring in CP_FileType, I_FileTypeLen, I_FileNameLen, CP_FileTypeComp,CPP CPP_FileNameMatch, ACP filename, k
    
void FileNameCompare(void *Pointer) {    
    struct S_FileTracking *FTinit = Pointer;

    //finds relevant files, by discarding any file thats overall length (including extension) is shorter than the relevent extension, than taking all remaining potential matches, and comparing the relevant last characters to specified extension
    
    char AC_FileType[]=".waifu";//menu config file (relevent extension)
	FTinit->CP_FileType=malloc(sizeof(AC_FileType));           //menu config file (relevent extension)
	strcpy(FTinit->CP_FileType, AC_FileType);
	FTinit->I_FileTypeLen = strlen(FTinit->CP_FileType);
    
    
    int k=0;
    int i=FTinit->TEMPCTR;      //STOPGAP UNTIL YOU REDO ALL YOUR LOOP VARS
	for (int j=0; j < i; j++)
	{                
        int I_FileNameLen = strlen(FTinit->CPP_FileName[j]);

		if (I_FileNameLen > FTinit->I_FileTypeLen)
		{
			char *ACP_FileTypeComp[FTinit->I_FileTypeLen];
			int I_FileNameOffset=(I_FileNameLen - FTinit->I_FileTypeLen);
            
            /*INVESTIGATE USING THE %s PRECISION OPERATOR HERE INSTEAD, FOR MORE SIMPLICITY?*/
			sprintf(ACP_FileTypeComp,"%s", (FTinit->CPP_FileName[j] + I_FileNameOffset));

			int I_StrComp=strcmp(ACP_FileTypeComp,FTinit->CP_FileType);

			if (I_StrComp == 0)
			{
                FTinit->CPP_FileNameMatch[k]=malloc(sizeof(FTinit->CPP_FileName[j]));
				FTinit->CPP_FileNameMatch[k]=FTinit->CPP_FileName[j];
                k++;
			}
		}
	}
	
	FTinit->I_TotalWindowGroups=k;
	
	FTinit->CPP_FileNameMatch = realloc( FTinit->CPP_FileNameMatch, (sizeof(void*) * k ));  //CPP_FileNameMatch gets allocated to handle ALL possible matches, but can be reallocated to only what does match

}    
    
    //bring in file depth counter, file tack counter, k
void FileDepthCounter(void *Pointer) {
    struct S_FileTracking *FTinit = Pointer;

//FILE DEPTH COUNTER (for sorting)
//really slick setup. runs through the characters of each string, outputs the number of matching '-' (the depth indicators in my filenames) into an integer array.

FTinit->IP_FileDepthCounter=malloc(sizeof(int) * FTinit->I_TotalWindowGroups);
FTinit->IP_FileTackCounter=malloc(sizeof(int) * FTinit->I_TotalWindowGroups);

for (int l=0; l < FTinit->I_TotalWindowGroups; l++)
{
    FTinit->IP_FileTackCounter[l]=0;                       //doesn't necessarily initialize to 0, so here it is.
    for (int m=0; FTinit->CPP_FileNameMatch[l][m]; m++)
    {
        FTinit->IP_FileTackCounter[l] += (FTinit->CPP_FileNameMatch[l][m] == '-');         //for some reason, trying ot mess with IP_FileDepthCounter after this produces a seg fault, and idk why. result of how += works?
    }
            FTinit->IP_FileDepthCounter[l]=(FTinit->IP_FileTackCounter[l] + 1);
}

}

//bring in k, FileDepthCounter, IP_FileTackCounter, CPP_FileNameMatch
void FileDepthSorter (void *Pointer) {
//bubble sorting the collected filenames, from deepest window to shallowest. necessary for parent-child inheritance in window launcher
    struct S_FileTracking *FTinit = Pointer;

    int k=FTinit->I_TotalWindowGroups;
    
    for (int c = 0; c < k-1; c++)               
    {
    
        for (int d = 0, I_TempBuff; d < k-c-1; d++)
        {   
           
            if (FTinit->IP_FileDepthCounter[d] < FTinit->IP_FileDepthCounter[d+1])
            {
                I_TempBuff = FTinit->IP_FileTackCounter[d];
                FTinit->IP_FileTackCounter[d] = FTinit->IP_FileTackCounter[d+1];
                FTinit->IP_FileTackCounter[d+1] = I_TempBuff;
                
                I_TempBuff = FTinit->IP_FileDepthCounter[d];
                FTinit->IP_FileDepthCounter[d] = FTinit->IP_FileDepthCounter[d+1];
                FTinit->IP_FileDepthCounter[d+1] = I_TempBuff;
                
                char *Temp=FTinit->CPP_FileNameMatch[d];
                FTinit->CPP_FileNameMatch[d]=FTinit->CPP_FileNameMatch[d+1];
                FTinit->CPP_FileNameMatch[d+1]=Temp;
            
            }
        }
    }
}
    
    //stripping the filenames of extensions (the filenames themselves hold information regarding window positioning)
void FileNameStripper (void *Pointer) {
        struct S_FileTracking *FTinit = Pointer;

    int k=FTinit->I_TotalWindowGroups;
    FTinit->CPP_FileNameStripped=malloc(sizeof(void*) * k);

    for (int e=0, I_Temp; e < k; e++)           //strips the file names
    {
        
        I_Temp=sizeof(FTinit->CPP_FileNameMatch[e]);
        
        FTinit->CPP_FileNameStripped[e]=malloc(sizeof(FTinit->CPP_FileNameMatch[e]));
        
        strcpy(FTinit->CPP_FileNameStripped[e], FTinit->CPP_FileNameMatch[e]);
        
        strtok(FTinit->CPP_FileNameStripped[e], ".");
        
        FTinit->CPP_FileNameStripped[e]=realloc(FTinit->CPP_FileNameStripped[e], ( I_Temp - (FTinit->I_FileTypeLen * sizeof(char))));    
        //i only need the half up to the null where the . used to be
        
    }
}

void LaunchRoutine(void *PointerOne, void *PointerTwo) { 
    
    struct S_FileTracking *FTinit = PointerOne;
    struct S_WinChar *WCinit = PointerTwo;
    struct S_FileContents FCinit[FTinit->I_TotalWindowGroups];  //take note, this is local!
    
    int k=FTinit->I_TotalWindowGroups;
    
    const int I_LayerOffset=2;
    char *OpenFileIndex[k+1];//+1 because i dont have root menu options made yet (it doesn't get read to increment k). but im still simulating it below. so it needs t eroom/

    OpenFileIndex[0]=malloc(sizeof(char) * 11);
    strcpy(OpenFileIndex[0],"root.waifu");
    FTinit->I_WindowGroupCtr++;
    
    
    for (int a=0; a < k; a++)
    {
        char *DynamicFileOrder[FTinit->IP_FileDepthCounter[a]];          //this is how many layers down the order will have to go
        char *DynamicFileOrderStripped[FTinit->IP_FileDepthCounter[a]];

        //runs down every parent window group of a window group, and provides stripped and unstripped version of them
        for (int b=0; b < FTinit->IP_FileDepthCounter[a]; b++)
        {
            DynamicFileOrder[b]=malloc( (sizeof(FTinit->CPP_FileNameMatch[a]) - (sizeof(char) * ( FTinit->IP_FileTackCounter[a] - b) * I_LayerOffset )));
            sprintf(DynamicFileOrder[b], "%.*s%s", (strlen(FTinit->CPP_FileNameStripped[a]) - ( ( FTinit->IP_FileTackCounter[a] - b) * I_LayerOffset ) ), FTinit->CPP_FileNameStripped[a], FTinit->CP_FileType );
            DynamicFileOrderStripped[b]=malloc( (sizeof(FTinit->CPP_FileNameStripped[a]) - (sizeof(char) * ( FTinit->IP_FileTackCounter[a] - b) * I_LayerOffset )));
            sprintf(DynamicFileOrderStripped[b], "%.*s", (strlen(FTinit->CPP_FileNameStripped[a]) - ( ( FTinit->IP_FileTackCounter[a] - b) * I_LayerOffset ) ), FTinit->CPP_FileNameStripped[a] );
        }
        
        for (int c=0; c < FTinit->IP_FileDepthCounter[a]; c++)          //something is getting fucked up here
        {
            
            int RETURNPLACEHOLDER;
            //compares current file to all previously opened files
            //THIS IS A FUNCTION! but not anymore?
        
            int d;
            for (d=0; d < FTinit->I_WindowGroupCtr; d++)
            {
                if(strcmp(DynamicFileOrder[c],OpenFileIndex[d]) == 0)
                {
                    RETURNPLACEHOLDER=d;//return (d);       //for a redundnat file-- giving its original window group
                    break;  //if i find a match before its done, i stop thet loop! otherwise returnplaceholder wil lalways be 0 as it goes past and isnt' true anymore 
                }
            
                else
            
                {
                    RETURNPLACEHOLDER=-1;//im negative integer because matches can occur anywhere d -ge 0
                }
            }

            char *TempToken;

            if (c+1 < FTinit->IP_FileDepthCounter[a])
            {
                char *TempString;
                TempString=malloc(sizeof(char) * (strlen(DynamicFileOrderStripped[c+1]) + 1));
                strcpy(TempString, DynamicFileOrderStripped[c+1]);
                
                TempToken=strtok(TempString, "-");

                for (int n=0; n > c+1; ++n)
                {
                    TempToken=strtok(NULL, "-");
                }
            }
            
            int TempTokenInt=atoi(TempToken);
 
           int I_HistoryCheck=RETURNPLACEHOLDER;         //FileHistory();
            if (I_HistoryCheck != -1)   //im a negative number, because its possible to get a  match in the d=0 position above (guaranteed actually, since root window group occupies that space)
            {
                WCinit[c].I_WindowGroup=I_HistoryCheck;
                continue;
              
            }

            WCinit[c].I_WindowGroup=FTinit->I_WindowGroupCtr;   //global i
            WCinit[c].I_ParentGroup=( c==0 ? c : WCinit[c-1].I_WindowGroup ); //if this is the first iteration of a loop (so the first windows off root), let the parent be root. otherwise let it be the window before it.

            OpenFileIndex[FTinit->I_WindowGroupCtr]=malloc(sizeof(DynamicFileOrder[c]));
            strcpy(OpenFileIndex[FTinit->I_WindowGroupCtr], DynamicFileOrder[c]);                            
            
            FileParser(DynamicFileOrder[c], &FCinit[FTinit->I_WindowGroupCtr]);
            
            SetWindowSize(&WCinit[c]);
            XYCoordinates(DynamicFileOrderStripped[c], &WCinit[c], c);

            int l;
            for (l=0; l < FCinit[FTinit->I_WindowGroupCtr].I_FileTotalLines; l++)   //make this loop a function, put it into the (Skip) iteration too?)
            {
                WCinit->I_RetrogradeWindowPos=l;
                
                if(FCinit[FTinit->I_WindowGroupCtr].IP_FileLineItemType[l] == 0)
                {
                    printf("Menu Item Position %d Launched--------------------------------------------\n",l); //LaunchEm
                }
                
                if((FCinit[FTinit->I_WindowGroupCtr].IP_FileLineItemType[l] == 1) && (TempTokenInt == l))
                {
                    printf("Sub Menu Item Position %d Launched------------------------------------------\n",l); //LaunchEm (Submenu type). this same if willl go in the alternate path too.
                    //A thought about submenus: because of how its set up, a submenu launches ONLY when its going to launch its child group next. so i can just import the entire structure, and use the WindowGroup id of the next launched window group, which will just be WCinit[ Current + 1 ]->I_WindowGroup
                }

                //if() if its submenu type AND the next depth layer is = to l, than do the right launch
                
                printf("loop %d\n", l);
  
            }
            
            //printf("%s - Window Group: %d - Parent Group: %d\n",OpenFileIndex[FTinit->I_WindowGroupCtr],WindowGroup[c],ParentGroup[c]);
            FTinit->I_WindowGroupCtr++;
        }
    }
}
    
//FTinit->I_WindowGroupCtr, DymamicFileOrder[c], struct for FCinit
void FileParser(char *DynamicFileOrder, void *Pointer){

struct S_FileContents *FCinit = Pointer;

//create the FILEDIRECT by using CPP_RaikaFileDir and current DynamicFilename.

char *CP_FileDirect;
CP_FileDirect=malloc(strlen(CPP_RaikaFileDir) + ( strlen(DynamicFileOrder) + 1) * sizeof(char)); //this allocates exact because it only includes a single null byte
sprintf(CP_FileDirect, "%s%s", CPP_RaikaFileDir, DynamicFileOrder);

FILE *fp;
FILE *fpl;

FCinit->I_FileTotalLines=0;
int I_LongestLine=0;


fp=fopen(CP_FileDirect, "r");
		
if (fp == NULL)
{
    exit (1); //error handling here
}
        
for (int i=1, j=0; i != EOF; i=fgetc(fp), j++)
{
    FCinit->I_FileTotalLines += (i == '\n');
    if(i == '\n')
    {
        I_LongestLine = ( j > I_LongestLine ? j : I_LongestLine ); 
        j=0;
    }
}

fclose(fp);

I_LongestLine++;    //ive gotta use this as a buffer later

rewind(fp);

int k=0;
char TEMP[I_LongestLine];

FCinit->IP_FileLineItemType=malloc(sizeof(int) * FCinit->I_FileTotalLines);
FCinit->CPP_FileLineItemName=malloc(sizeof(void*) * FCinit->I_FileTotalLines);
FCinit->CPP_FileLineItemAction=malloc(sizeof(void*) * FCinit->I_FileTotalLines);

fopen(CP_FileDirect, "r");

        while (fgets(TEMP, (sizeof(char) * (I_LongestLine)), fp) != NULL && k < FCinit->I_FileTotalLines)    
        {

                    TEMP[strcspn(TEMP, "\n")]=0;  //really cool. strcspn gives the index position of the \n, and TEMP[] goes to the character elmeent, replacing it with a 0 aka null/ end of array
                    
                    char *check;
                    check=strtok(TEMP, "*");
                    
                    for(int i=0; check != NULL; check=strtok(NULL, "*"), i++)
                    {
                        //printf("%s\t",check);

                        if (i==0)
                        {
                            FCinit->IP_FileLineItemType[k]=atoi(check);
                            //printf("Action Item: %d\n", FCinit->IP_FileLineItemType[k]);

                        }
                        
                        if (i==1)
                        {
                            FCinit->CPP_FileLineItemName[k]=malloc(sizeof(char) * (strlen(check)+1));                            
                            strcpy(FCinit->CPP_FileLineItemName[k],check);
                            //printf("Name: %s\n", FCinit->CPP_FileLineItemName[k]);

                        }   
                        
                        if (i==2)
                        {
                            FCinit->CPP_FileLineItemAction[k]=malloc(sizeof(char) * (strlen(check)+1));  
                            strcpy(FCinit->CPP_FileLineItemAction[k], check);    
                            //printf("Action Item: %s\n", FCinit->CPP_FileLineItemAction[k]);
                        }
                    }
                    
                    k++;
        }
        
                free(CP_FileDirect);
fclose(fp);

}

void SetWindowSize(void *Pointer){   //after the file parsing, so i could reference that info and make the sizes dynamic
    struct S_WinChar *WCinit = Pointer;

    WCinit->I_Width=200;
    WCinit->I_Height=50;
}

void XYCoordinates(char *DynamicFileOrderStripped, void *Pointer, int c){
    
    struct S_WinChar *WCinit = Pointer;
    
    char *CPP_FileNameInt;

    CPP_FileNameInt=malloc(sizeof(char) * (strlen(DynamicFileOrderStripped) + 1));

    strcpy(CPP_FileNameInt, DynamicFileOrderStripped);
    
    WCinit->I_Ypos=0;
    
    for(char *TEMP=strtok(CPP_FileNameInt, "-"); TEMP != NULL; TEMP=strtok(NULL, "-"))
    {
        WCinit->I_Ypos+=atoi(TEMP);
    }
    WCinit->I_Xpos=c+1;
    //printf("X Pos: %d in %s\nY Pos: %d in %s\n", WCinit->I_Xpos, DynamicFileOrderStripped, WCinit->I_Ypos, DynamicFileOrderStripped);

}

void MenuItemThread(void *Pointer){
    struct S_WinChar *WCinit = Pointer;
    int I_Ypos=WCinit->I_Ypos;   
}
