#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>

int main(int argc, char *argv[]){
  char* sopath = "./logger.so";
  char* output_file = NULL;
  char* config = argv[1];
  char **command;
  int opt;
  int fd = -1;
  int index = 2;
  opterr = 0;
  while((opt = getopt(argc, argv, "p:o:")) != -1){
    if(opt == '?'){
      break;
    }
    switch(opt){
      case 'p':
        index +=2;
        sopath = optarg;
        break;
      case 'o':
        index+=2;
        output_file = optarg;
        break;
    }
  }
  // for(int i = 0; i < argc; i++){
  //   printf("argv[%d]: %s\n", i, argv[i]);
  // }
  // printf("argv[index]: %s\n", argv[index]);
  // printf("argc: %d, optind: %d, index: %d\n", argc, optind, index);

  // return 0;
  setenv("LD_PRELOAD", sopath, 1); // set LD_PRELOAD to the path of the shared object
  setenv("CONFIG", config, 1);

  if(output_file != NULL){
    fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if(fd == -1){
      perror("open");
      return EXIT_FAILURE;
    }
    dup2(fd, STDERR_FILENO);
  }

  command = &argv[index];
  execvp(command[0], command);
  
  return 0;
}