#include <sys/stat.h>
#include <time.h>
#include "csapp.h"


int file_size(char *fn) {
    struct stat s; 
    if (stat(fn, &s) == 0)
        return s.st_size;
    return 0; 
}

time_t file_date_fd(int fd){
  struct stat s; 
  fstat(fd, &s);
  return s.st_mtime;
}

int max(int a,int b){
  if(a<b) return b;
  return a;
}

void printProgress(double percentage) {
    int val = (int) percentage;
    int lpad = (int) percentage;
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}