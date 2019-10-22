#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>
#include "./headers/functions.h"


volatile sig_atomic_t counterFork = 0;
pid_t ppid;

void signalHandler(){
  //fail transfer
  signal(SIGUSR1,signalHandler);
  //alarm
  signal(SIGALRM, signalHandler);
  counterFork++;
}

void traverse(char *fn, int indent, char* writerFifo, char* mDir, char* logFile, char* id,int bufferSize){
  DIR *dir;
  struct dirent *entry;
  char path[Max_Path];
  struct stat info;
  int flag=0;
  if ((dir = opendir(fn)) == NULL){
    perror("opendir() error");
    kill(ppid,SIGUSR1);
    exit(0);
  }
  else {
    while ((entry = readdir(dir)) != NULL) {
      if (entry->d_name[0] != '.'){
        strcpy(path, fn);
        strcat(path, "/");
        strcat(path, entry->d_name);
        if (stat(path, &info) != 0){
          perror("error");
          fprintf(stderr, "stat() error on %s: %s\n", path,
                  strerror(errno));
          kill(ppid,SIGUSR1);
          exit(0);
        }
        else if (S_ISDIR(info.st_mode)){
            traverse(path, indent+1, writerFifo, mDir, logFile, id, bufferSize);
        }
        else if(S_ISREG(info.st_mode)){
        //we found a file
        //write to pipe
          int fd,fptr;
          unsigned short nlen=0;
          unsigned int flen=0;
          struct stat info;
          FILE *lg;
          char* buffer = malloc(bufferSize + 1);
          nlen = strlen(path);

          fd = open(writerFifo, O_WRONLY);
          if(fd < 0){
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }
          if(write(fd,&nlen, 2) < 0){
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }

          if(write(fd,path, nlen + 1) < 0){
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }
          //open file
          fptr = open(path,O_RDONLY);
          if(!fptr){
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }
          //find the size of the file
          flen = findSize(path);

          if(write(fd,&flen, 4) < 0){
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }

          int writeBytes = 0;
          //read from file and write to fifo
          while(flen > 0){

            if(flen < bufferSize){

              read(fptr,buffer,flen);

              writeBytes += flen;

              buffer[flen] = '\0';
              if(write(fd,buffer,flen) < 0){
                perror("error");
                kill(ppid,SIGUSR1);
                exit(0);
              }
              flen = 0;
              break;
            }

            writeBytes += bufferSize;

            read(fptr,buffer,bufferSize);
            buffer[bufferSize] = '\0';
            if(write(fd,buffer,bufferSize) < 0){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            flen -= bufferSize;
        }

          lg = fopen(logFile, "a");
          if(!lg){
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }
          fprintf(lg,"Write %d\n",writeBytes);
          fclose(lg);

          close(fd);
          free(buffer);
          close(fptr);
        }
      }
    }
    closedir(dir);
  }
}

void checkDir(char* id,char* cDir, char* mDir, char* logFile, int fileDescriptor, int bufferSize){
  DIR *d;
  struct dirent *dir;
  d = opendir(cDir);
  const char *dot;
  int n,s=0;
  char* nid = malloc(3);
  n = atoi(id);
  char* writerFifo = malloc(Max_Path);
  char* readerFifo = malloc(Max_Path);
  char* inputFile = malloc(Max_Path);
  if(d){
   while ((dir = readdir(d)) != NULL) {
     dot = strrchr(dir->d_name, '.');
     sscanf(dir->d_name,"%d.id",&s);
     //if we find a file with different name than the current client id
     if(n!=s){
       if(strncmp(dot,".id",3) == 0){
         //make the fifo file NAMES
         sprintf(writerFifo, "%s/%d_to_%d.fifo",cDir,n,s);
         sprintf(readerFifo, "%s/%d_to_%d.fifo",cDir,s,n);
         //make the inputFile directory name
         sprintf(inputFile, "%d_input",n);

         //find the files and the path and create processes
         sprintf(nid,"%d",s);
         createPipes(inputFile,writerFifo,readerFifo,mDir,logFile,id,bufferSize,nid,cDir,fileDescriptor);
      }
     }
   }
   closedir(d);
  }
  free(nid);
  free(writerFifo);
  free(readerFifo);
  free(inputFile);
}

void createPipes(char* inputFile,char* writerFifo, char* readerFifo, char* mDir, char* logFile, char* id, int bufferSize,char* nid, char* cDir,int fileDescriptor){
  signal(SIGUSR1,signalHandler);

  struct stat info;
  pid_t pid, wpid;
  int status = 0;
  char wFifo[100];
  char rFifo[100];
  strcpy(wFifo,"");
  strcpy(rFifo,"");
  for(int kid=0; kid<2; kid++){
    pid = fork();
    if(pid < 0){
      perror("error");
      kill(ppid,SIGUSR1);
      exit(0);
    }
    else if(pid == 0){
      //WRITER
      if(kid == 0){
        strcpy(wFifo,writerFifo);
        //check if fifo already exists
        if(lstat(wFifo,&info) !=0 ){
          if(errno == ENOENT) {
            //  doesn't exist
            mkfifo(wFifo,0666);
          }
          else{
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }
        }
        //it exists
        traverse(inputFile, 0, wFifo,mDir,logFile,nid,bufferSize);
        int fd = open(wFifo, O_WRONLY);

        if(fd < 0){
          perror("error");
          kill(ppid,SIGUSR1);
          exit(0);
        }
        unsigned short zero = 0;

        if(write(fd, &zero, 2) < 0){
          perror("error");
          kill(ppid,SIGUSR1);
          exit(0);
        }


        close(fd);
        exit(1);
      }
      //READER
      else if(kid == 1){
        strcpy(rFifo,readerFifo);
        signal(SIGALRM, signalHandler);
        int fd,fp;
        struct stat info;
        unsigned short len=0;
        unsigned int flen=0,temp=0;
        FILE *lg,*newF;
        char* b ,*f;
        char outputFile[Max_Path];
        char dirName[Max_Path];
        char* buffer;
        if(lstat(rFifo,&info) !=0 ){
          if(errno == ENOENT) {
            //  doesn't exist
            mkfifo(rFifo,0666);
          }
          else{
            perror("error");
            kill(ppid,SIGUSR1);
            exit(0);
          }
        }

        fd = open(rFifo, O_RDONLY);

        if(fd < 0){
          perror("error");
          kill(ppid,SIGUSR1);
          exit(0);
        }
        char fileN[Max_Path];
        while(1){
            buffer = malloc(bufferSize + 1);
            alarm(30);
            if(read(fd,&len,2) < 0){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            alarm(0);

            if(len == 0){
              close(fd);
              free(buffer);
              unlink(rFifo);
              exit(2);
            }

            alarm(30);
            if(read(fd,fileN,len + 1) < 0){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            alarm(0);
            //get the file name
            b = strrchr(fileN,'/');
            b++;
            f = strchr(fileN, '/');
            f++;
            strcpy(dirName,"");
            sprintf(dirName, "%s/%s/%s",mDir,nid,f);

            //find the size of the file
            alarm(30);
            if(read(fd,&flen,4) < 0){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            alarm(0);


            //write in the logFile
            lg = fopen(logFile, "a+");
            if(!lg){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            fprintf(lg,"Name  %s\n",b);
            fclose(lg);

            temp = flen;
            //get the file path inside mirror dir
            strcpy(outputFile,dirName);
            //Read from fifo and write to mirror

            makeDirectory(dirName,b);

            fp = open(outputFile, O_WRONLY | O_APPEND | O_CREAT, 0644);
            if(fp<0){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            int readBytes = 0;
            while(temp > 0){

              if(temp < bufferSize){
                alarm(30);
                if(read(fd,buffer,temp) < 0){
                  perror("error");
                  kill(ppid,SIGUSR1);
                  exit(0);
                }
                alarm(0);
                readBytes += temp;
                write(fp,buffer,temp);
                temp = 0;
                break;
              }

              readBytes += bufferSize;
              alarm(30);
              if(read(fd,buffer,bufferSize) < 0){
                perror("error");
                kill(ppid,SIGUSR1);
                exit(0);
              }
              alarm(0);

              //write to file
              if(write(fp,buffer,bufferSize) < 0){
                perror("error");
                kill(ppid,SIGUSR1);
                exit(0);
              }
              temp -= bufferSize;

            }
            lg = fopen(logFile, "a");
            if(!lg){
              perror("error");
              kill(ppid,SIGUSR1);
              exit(0);
            }
            fprintf(lg,"Read %d\n",readBytes);
            fclose(lg);
            close(fp);
            free(buffer);
          }
      }
    }
  }
  while((wpid = wait(&status)) > 0){
    int exitStatus = WEXITSTATUS(status);
    if(exitStatus == 2){
        printf("I'm parent process, and just received 00\n");
    }
    else if(exitStatus == 0){
      // if(counterFork <= 3){
        printf("Fail on transfer.Trying again...\n");
        // checkDir(id,cDir,mDir,logFile,fileDescriptor,bufferSize);
      // }
    }
  }
}

void makeDirectory(char* fileName, char* file){
  char *a;
  char fName[Max_Path];
  char dirName[Max_Path];
  int status;
  strcpy(fName,fileName);
  a = strtok(fName, "/");

  sprintf(dirName, "%s",".");
  while((a = strtok(NULL, "/")) != NULL){
    if(strcmp(a,file) != 0){
      sprintf(dirName, "%s/%s",dirName,a);
    }
  }
  _mkdir(dirName);
}

static void _mkdir(const char *dir) {
        char tmp[Max_Path];
        char *p = NULL;
        size_t len;

        snprintf(tmp, sizeof(tmp),"%s",dir);
        len = strlen(tmp);
        if(tmp[len - 1] == '/'){
          tmp[len - 1] = 0;
        }
        for(p = tmp + 1; *p; p++)
          if(*p == '/') {
            *p = 0;
            mkdir(tmp, S_IRWXU);
            *p = '/';
          }
        mkdir(tmp, S_IRWXU);
}


long int findSize(char* fileName){
  // opening the file in read mode
  FILE* fp = fopen(fileName, "r");

  // checking if the file exist or not
  if (fp == NULL){
    perror("error");
    kill(ppid,SIGUSR1);
    exit(0);
  }

  fseek(fp, 0L, SEEK_END);

  // calculating the size of the file
  long int res = ftell(fp);

  // closing the file
  rewind(fp);
  fclose(fp);

  return res;
}
