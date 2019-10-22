#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <fcntl.h>
#include <sys/inotify.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include "./headers/functions.h"
#include "./headers/inotifyFunctions.h"

char mirror[Max_Path];
char common[Max_Path];
char logF[Max_Path];

void signalDirectory(){
  pid_t pid;
  FILE *lg;
  pid = fork();
  if(pid == 0){
    execl("/bin/rm","rm","-rf",mirror,NULL);
  }

  int status = 0;
  while(wait(&status) > 0);
  lg = fopen(logF, "a");
  fprintf(lg,"Left\n");
  fclose(lg);
  unlink(common);
  exit(1);
}

int main(int argc, char *argv[]){

  int id, cDir, iDir, mDir, bufferSize, logFile, filedescriptor,b;
  DIR* dir;
  FILE *fp;
  struct stat buf;
  char pathFile[PATH_MAX];
  FILE *lg;

  if(argc != 13){
    printf("Some arguments are missing\n");
    return 0;
  }
  //Handle the arguments from the command line
  for(int arg = 1; arg < argc; arg++){
    //if we are not at the last arg
    if(arg < argc - 1){
      if(strncmp(argv[arg], "-n", 2) == 0){
        id = arg + 1;
      }
      else if(strncmp(argv[arg], "-c", -2) == 0){
        cDir = arg + 1;
      }
      else if(strncmp(argv[arg], "-i", -2) == 0){
        iDir = arg + 1;
      }
      else if(strncmp(argv[arg], "-m", -3) == 0){
        mDir = arg + 1;
      }
      else if(strncmp(argv[arg], "-b", -3) == 0){
        b = arg + 1;
      }
      else if(strncmp(argv[arg], "-l", -2) == 0){
        logFile = arg + 1;
      }
    }
  }

  dir = opendir(argv[iDir]);
  if (dir){
    closedir(dir);
  }
  else if (ENOENT == errno){
    closedir(dir);
    printf("Input dir does not exist\n");
    return 0;
  }
  else{
    closedir(dir);
    printf("Error while opening input dir\n");
  }

  dir = opendir(argv[mDir]);
  if (dir){
    closedir(dir);
    printf("Mirror dir exists\n");
    return 0;
  }
  else if (ENOENT == errno){
    mkdir(argv[mDir], 0700);
    closedir(dir);
  }
  else{
    closedir(dir);
    printf("Error while opening mirror dir\n");
  }

  dir = opendir(argv[cDir]);
  if (dir){
    sprintf(pathFile, "%s/%s.id",argv[cDir],argv[id]);
    if(stat(pathFile, &buf) != 0 && errno == ENOENT){
      filedescriptor = open(pathFile,O_CREAT);
      fp = fopen(pathFile, "w+");
      fprintf(fp, "%d\n",getpid());
      closedir(dir);
      fclose(fp);
    }
    else{
      printf("File exists\n");
      closedir(dir);
      return 0;
    }
  }
  else if (ENOENT == errno){
    mkdir(argv[cDir], 0700);
    closedir(dir);
  }
  else{
    closedir(dir);
    printf("Error while opening common dir\n");
  }

  strcpy(mirror,argv[mDir]);
  sprintf(common,"%s/%s.id",argv[cDir],argv[id]);
  strcpy(logF,argv[logFile]);

  signal(SIGINT,signalDirectory);
  signal(SIGQUIT,signalDirectory);

  //write in the logFile
  lg = fopen(argv[logFile], "a");
  if(!lg){
    perror("error");
    exit(0);
  }
  fprintf(lg,"My %s\n",argv[id]);
  fclose(lg);

  bufferSize = atoi(argv[b]);
  checkDir(argv[id],argv[cDir],argv[mDir],argv[logFile],filedescriptor,bufferSize);

  inotifyFun(argv[cDir],argv[mDir],argv[logFile],filedescriptor,argv[id],bufferSize);

  return 0;
}
