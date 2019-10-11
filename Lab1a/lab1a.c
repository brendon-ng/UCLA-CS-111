
//NAME: Brendon Ng
//EMAIL: brendonn8@gmail.com
//ID: 304925492

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

int call_command(int argc, char* argv[], int* optind, int fdc, int fds[], int verbose){
  int subargc = 0;
  int parseind = *optind;
  if(verbose)
    printf("--command");
  while(parseind < argc){
    int doubledash = 0;
    for(int i = 0; argv[parseind][i] != '\0'; i++){
      if(argv[parseind][i] != '-')
	break;
      doubledash+=1;
      if(doubledash == 2)
	break;
    }
    if(doubledash == 2)
      break;

    if(verbose)
      printf(" %s", argv[parseind]);
    subargc += 1;
    parseind += 1;
  }

  if(verbose)
    printf("\n");
  
  if(subargc < 4){
    fprintf(stderr, "Error: Not enough arguments for --command\nUsage: --command i o e cmd args\n");
    return 1;
  }

  int index = atoi(argv[*optind]);
  if(index >= fdc){
    fprintf(stderr, "Error: wrong fd index: %d - Out of Bounds\n",index);
    return 1;
  }
  int in = fds[index];
  *optind+=1;
  index = atoi(argv[*optind]);
  if(index >= fdc){
    fprintf(stderr, "Error: wrong fd index: %d - Out of Bounds\n",index);
    return 1;
  }
  int out = fds[atoi(argv[*optind])];
  *optind+=1;
  index = atoi(argv[*optind]);
  if(index >= fdc){
    fprintf(stderr, "Error: wrong fd index: %d - Out of Bounds\n",index);
    return 1;
  }
  int err = fds[atoi(argv[*optind])];
  *optind+=1;
  
  char* subargs[subargc - 2];
  for(int i=0; i<subargc-3; i++){
    subargs[i] = argv[*optind];
    *optind+=1;
  }
  subargs[subargc-3] = NULL;

  int PID = fork();
  if(PID < 0){
    fprintf(stderr, "Error: fork failed");
    return 1;
  }
  if(PID == 0){
    dup2(in,0);
    close(in);
    dup2(out,1);
    close(out);
    dup2(err,2);
    close(err);
    execvp(subargs[0], subargs);
    return 1;
  }
  
  return 0;
}

int main(int argc, char* argv[]){
  int exitstatus = 0;
  int fileDescriptors[200];
  int fdindex = 0;
  
  static struct option args[] = {
				 {"rdonly", 1, NULL, 'r'},
				 {"wronly", 1, NULL, 'w'},
				 {"command", 1, NULL, 'c'},
				 {"verbose", 0, NULL, 'v'},
				 {0,         0,    0,  0}
  };

  int opt, ret;
  int verbose = 0;
  
  while((opt=getopt_long(argc, argv, "", args, NULL)) != -1) {
    if(opt == '?'){
      fprintf(stderr, "Error: Unrecognized argument\nUsage: ./simpsh --rdonly [file] --wronly [file] --command [stdin] [stdout] [stderr] [cmd] [args] --verbose\n");
      exitstatus = 1;
    }

    switch(opt){
    case 'r':
      if(verbose)
	printf("--rdonly %s\n", optarg);

      fileDescriptors[fdindex] = open(optarg, O_RDONLY);
      if(fileDescriptors[fdindex] < 0 ){
	fprintf(stderr, "Error #%d: Error opening file %s\n%s\n",
		errno, optarg, strerror(errno));
	exitstatus = 1;
      }
      fdindex += 1;
      break;
    case 'w':
      if(verbose)
	printf("--wronly %s\n", optarg);
      fileDescriptors[fdindex] = open(optarg, O_WRONLY);
      if(fileDescriptors[fdindex] < 0 ){
        fprintf(stderr, "Error #%d: Error opening file %s\n%s\n",
                errno, optarg, strerror(errno));
        exitstatus = 1;
      }
      fdindex += 1;
      break;
    case 'c':
      optind -= 1;
      ret = call_command(argc, argv, &optind, fdindex, fileDescriptors,verbose);
      if(ret != 0)
	exitstatus = 1;
      break;
    case 'v':
      verbose = 1;
      break;
    }
  }

  exit(exitstatus);

}
