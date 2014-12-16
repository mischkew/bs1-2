/* Betriebssystem & Middleware
 *
 * Betriebssysteme I WS 2014/2015
 *
 * Uebung 2.5
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef UNICODE
#pragma message("!!! Unicode is enabled. Be carefull using getDescription with none-UNICODE strings. !!!")
#endif
#define XSIZE 500
#define YSIZE 500
#include "algorithm.h"

// Thread Rountine
DWORD WINAPI run(LPVOID *data) {
    // cast void pointer to integer value
    int *bins = (int*) data;
    int from = bins[0];
    int to = bins[1];
    int x, y, len;
    char bgr[3];
    long int bytepos;

    printf("Setup thread from %d to %d\n\n", from, to);

    for (y = from; y < to; y++) {
        for (x = 0; x < XSIZE; x++) {
            getColorValuesAt(x * (2.0 / XSIZE) - 1.5, y * (2.0 / YSIZE) - 1.0, &bgr[2], &bgr[1], &bgr[0]);
            bytepos = 3*((y * XSIZE) + x) + 54;

            // wait until other threads finished writing
            sem_wait(&sem);
            fseek(fd, bytepos, SEEK_SET);
            int len = fwrite(bgr,1,3,fd);
            if (-1 == len || len != 3) {
                perror("write");
                exit(4);
            }
            sem_post(&sem);
        }
    }
    free(data);
}

HANDLE Mutex;

struct individualThread{
    int* lines;
    int numberOfLines;
    FILE *threadPt;
};

DWORD WINAPI calculateColors(LPVOID lpParam){
    struct individualThread *thData = (struct individualThread*) lpParam;
    char bgr[3];
    long int bytepos;
    int x, y, len, i;
    DWORD dwWaitResult;
    BOOL bContinue;
    
    for (i = 0; i < thData->numberOfLines; i++) {
        y = thData->lines[i];
        for (x = 0; x < XSIZE; x++) {
            getColorValuesAt(x * (2.0 / XSIZE) - 1.5, y * (2.0 / YSIZE) - 1.0, &bgr[2], &bgr[1], &bgr[0]);
            bytepos = 3*((y * XSIZE) + x) + 54;
            bContinue = TRUE;
            
            while(bContinue){
                dwWaitResult = WaitForSingleObject(
                                                   Mutex,   			// handle to Mutex
                                                   INFINITE);           // zero-second time-out interval
                
                if(dwWaitResult == WAIT_OBJECT_0){
                    
                    __try{
                        bContinue = FALSE;
                        fseek(thData->threadPt, bytepos, SEEK_SET);
                        len = fwrite(bgr,1,3,thData->threadPt);
                        
                        if (-1 == len || len != 3) {
                            perror("write error in Thread");
                            exit(4);
                        }
                    }
                    __finally{
                        // Release ownership of the mutex object
                        if (! ReleaseMutex(Mutex))
                        {
                            perror("mutex error");
                            exit(5);
                        }
                    }
                    
                    
                }
                
                else if(dwWaitResult == WAIT_ABANDONED){
                    printf("Thread %d: wait timed out\n", GetCurrentThreadId());
                    exit(5);
                }
                
            }
        }
    }
    
    return 0;
}

int main(int argc, char *argv[])
{
    FILE *fd;
    int len,x,y;
    char *dsc;
    char bgr[3];
    short svalue;
    int   lvalue;
    unsigned char header[54],*ptr=&header[0];
    
    /** Own declarations **/

    // number of Threads given by console input
    int numberOfThreads = atoi(argv[1]);

    // Array of all Handles, allocate memory for given number of handles
    HANDLE* threadHandles[numberOfThreads]; // = (HANDLE*)malloc(numberOfThreads * sizeof(HANDLE));

    // Array of individualThread-Structs to pass data to each thread
    //struct individualThread* data = (struct individualThread*)malloc(numberOfThreads * sizeof(struct individualThread));
    //int maxCalcPerThread = ceil((float)XSIZE*YSIZE/(float)numberOfThreads);
    //int counter, lineNumber, i, j;
    // calculation is divided by linecount
    if (numberOfThreads > YSIZE) {
        numberOfThreads = YSIZE;
    }

    /**********************/
    
    
    /** Own initialization **/
    // initialize individualThread-struct-array "data" by allocating memory
    // for (j=0; j < numberOfThreads; j++) {
        // data[j].lines    = (int*)malloc(maxCalcPerThread * sizeof(int));
        //data[j].threadPt = (FILE)malloc(sizeof(FILE));
    // }
    
    // set individualThread´s data in order to make the threads work
    /*counter = 0;
    lineNumber = 0;
    for(i=0; i < numberOfThreads; i++){
        for(j=i; j < 500; j+=numberOfThreads){
            data[i].lines[counter] = j;
            counter++;
            lineNumber++;
        }
        data[i].numberOfLines = lineNumber;
        lineNumber = 0; counter = 0;
    }*/
    
    // TODO check for line-numbers of each thread (delete later)
    /*for(i=0;i<numberOfThreads;i++){
        for(j=0;j<data[i].numberOfLines;j++){
            printf("%d , ", data[i].lines[j]);
        }
        printf("\n");
    }
    printf("printing done\n");*/
    /**********************/
    
    /** Set Up Mutex **/
    /*Mutex = CreateMutex(NULL, FALSE, NULL);
    
    if (Mutex == NULL)
    {
        printf("CreateMutex error: %d\n", GetLastError());
        return 1;
    }*/
    /**********************/
    
    getDescription(NULL,&len);
    if(NULL==(dsc=(char*)malloc(sizeof(char)*len)))
    {
        perror("malloc");
        exit(1);
    }
    getDescription(&dsc,&len);
    printf("Calculate %s %d\n",dsc,getId());
    fd=fopen("test.bmp","wb+");
    if(NULL==fd)
    {
        perror("open"); exit(1);
    }
    
    svalue=0x4d42;
    memcpy(ptr,&svalue,2);//signatur
    ptr+=2;
    lvalue=XSIZE*YSIZE*3+54;
    memcpy(ptr,&lvalue,4); //filesize
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4);//reserved
    ptr+=4;
    lvalue=54;
    memcpy(ptr,&lvalue,4);//image offset
    ptr+=4;
    lvalue=40;
    memcpy(ptr,&lvalue,4);//size of header follows
    ptr+=4;
    lvalue=XSIZE;
    memcpy(ptr,&lvalue,4);//with of image
    ptr+=4;
    lvalue=YSIZE;
    memcpy(ptr,&lvalue,4); //height of image
    ptr+=4;
    svalue=1;
    memcpy(ptr,&svalue,2); //number of planes
    ptr+=2;
    svalue=24;
    memcpy(ptr,&svalue,2); //number of pixel
    ptr+=2;
    lvalue=0; //compression
    memcpy(ptr,&lvalue,4); //compression
    ptr+=4;
    lvalue=XSIZE*YSIZE*3;
    memcpy(ptr,&lvalue,4); //size of image
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //xres
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //yres
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //number of colortables
    ptr+=4;
    lvalue=0;
    memcpy(ptr,&lvalue,4); //number of important colors
    ptr+=4;
    
    len=fwrite(header,1,sizeof(header),fd); //write header
    
    if(-1==len || len!=sizeof(header))
    {
        perror("write");
        exit(2);
    }
    
    /*
     for(y=YSIZE-1;y>=0;y--)
     {
     for(x=0;x<XSIZE;x++)
     {
     getColorValuesAt(x * (2.0 / XSIZE) - 1.5, y * (2.0 / YSIZE) - 1.0,&bgr[2],&bgr[1],&bgr[0]);
     
     len=fwrite(bgr,1,3,fd);
     if(-1==len || len!=3)
     {
     perror("write");
     exit(4);
     }
     }
     //no padding required because 1500%4 =0
     }
     */
    
    for (i = 0; i < numberOfThreads; i++) {
        int from = i * chunkCount;
        int to  = (i+1) * chunkCount;
        int* data = malloc(2 * sizeof(int));

        printf("Setup init %d %d\n", from, to);

        if (i == threadcount - 1) {
            to = YSIZE;
        }

        data[0] = from;
        data[1] = to;

        printf("create a Thread \n");

        threadHandles[i] = CreateThread(NULL,
                                        0,
                                        run,
                                        data,
                                        0,
                                        NULL);

        if (NULL == threadHandles[i]) {
            printf("thread could not be created\n");
            exit(1);
        }
    }
    
    // wait for all threads to finish calculation in order to start writing to file
    for (i = 0; i < numberOfThreads; i++) {
        DWORD wait_status = WaitForSingleObject(threadHandles[i], INFINITE);
        if (wait_status === WAIT_FAILED) {
            printf("wait error\n");
            exit(1);
        }

        BOOL close_status = CloseHandle(threadHandles[i]);
        if (!close_status) {
            prinft("close error\n");
            exit(1);
        }
    }
    
    // CloseHandle(Mutex);
    
    fclose(fd);
    
}
