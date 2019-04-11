#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>
#include "./headers/inotifyFunctions.h"
#include "./headers/functions.h"

//function from inotify man page
void inotifyFun(char* cDir, char* mDir, char* logFile, int filedescriptor, char* id, int bufferSize){
  char buff;
  int fd, i;
  int wd;

  /* Create the file descriptor for accessing the inotify API */

  fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
     perror("inotify_init1");
     exit(0);
  }

  /* Mark directories for events
    - file was opened
    - file was closed */

     wd = inotify_add_watch(fd, cDir,
                               IN_CREATE | IN_DELETE);
     if (wd == -1) {
         fprintf(stderr, "Cannot watch '%s'\n", cDir);
         perror("inotify_add_watch");
         exit(0);
     }



  /* Wait for events and/or terminal input */

  while (1) {

             /* Inotify events are available */

             handle_events(fd, wd, mDir, cDir, logFile, filedescriptor, id, bufferSize);
  }

  /* Close inotify file descriptor */

  close(fd);
}

void handle_events(int fd, int wd, char* mDir, char* cDir, char* logFile, int fileDescriptor, char* id, int bufferSize) {
           /* Some systems cannot read integer variables if they are not
      properly aligned. On other systems, incorrect alignment may
      decrease performance. Hence, the buffer used for reading from
      the inotify file descriptor should have the same alignment as
      struct inotify_event. */

   char buf[4096]
       __attribute__ ((aligned(__alignof__(struct inotify_event))));
   const struct inotify_event *event;
   int i,n,s=0;
   ssize_t len;
   char *ptr;
   /* Loop while events can be read from inotify file descriptor. */

   for (;;) {

       /* Read some events. */

       len = read(fd, buf, sizeof buf);
       if (len == -1 && errno != EAGAIN) {
           perror("read");
           exit(0);
       }

       /* If the nonblocking read() found no events to read, then
          it returns -1 with errno set to EAGAIN. In that case,
          we exit the loop. */

       if (len <= 0){
          break;
       }

       /* Loop over all events in the buffer */

       for (ptr = buf; ptr < buf + len;
               ptr += sizeof(struct inotify_event) + event->len) {

         event = (const struct inotify_event *) ptr;

         /* Print the name of the file */
         if (event->len && event->mask & IN_CREATE){
           const char *dot = strrchr(event->name, '.');
           if(!dot || dot == event->name){
           }
           else{
             //check the extension of the file
             if(strncmp(dot,".id",3) == 0){
               if (wd == event->wd) {
                  char* writerFifo = malloc(14);
                  char* readerFifo = malloc(14);
                  char* inputFile = malloc(Max_Path);
                  char* nid = malloc(3);

                  n = atoi(id);
                  sscanf(event->name,"%d.id",&s);
                  //make the fifo file names
                  sprintf(writerFifo, "%s/%d_to_%d.fifo",cDir,n,s);
                  sprintf(readerFifo, "%s/%d_to_%d.fifo",cDir,s,n);
                  //make the inputFile directory name
                  sprintf(inputFile, "%d_input",n);

                  //find the files and the path and create processes
                  sprintf(nid,"%d",s);
                  // traverse(inputFile, 0, writerFifo,readerFifo,mDir,logFile,nid,bufferSize);
                  createPipes(inputFile,writerFifo,readerFifo,mDir,logFile,id,bufferSize,nid,cDir,fileDescriptor);

                  free(nid);
                  free(writerFifo);
                  free(readerFifo);
                  free(inputFile);
                }
             }
           }
         }
         //check for delete .id file
         else if(event->len && event->mask & IN_DELETE){
           const char *dot = strrchr(event->name, '.');
           if(!dot || dot == event->name){
           }
           else{
             if(strncmp(dot,".id",3) == 0){
               if (wd == event->wd) {
                 n=atoi(id);
                 sscanf(event->name,"%d.id",&s);
                 if(n != s){
                   pid_t pid = fork();
                   //if child process
                   if(pid == 0){
                     //delete directory
                     char path[Max_Path];
                     sprintf(path, "%d_mirror/%d/",n,s);
                     execl("/bin/rm","rm","-rf",path,NULL);
                  }
                  //parent process, just wait for the kid to finish
                    int status = 0;
                    while(wait(&status) > 0);

                 }
               }
             }
           }
         }
       }
   }
}
