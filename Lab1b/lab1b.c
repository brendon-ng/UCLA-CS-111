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
#include <signal.h>
#include <sys/wait.h>

struct Command{
  int pid;
  char* arguments[100];
  int exitStatus;
};

struct Command children[200];
int cchildren = 0;

int max(int a, int b){
  if(a>b)
    return a;
  else
    return b;
}

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

  if(in == -1 || out == -1 || err == -1){
    fprintf(stderr, "Error: attempting to access a closed file\n");
    return 1;
  }
  
  char* subargs[subargc - 2];
  for(int i=0; i<subargc-3; i++){
    subargs[i] = argv[*optind];
    children[cchildren].arguments[i] = argv[*optind];
    *optind+=1;
  }
  subargs[subargc-3] = NULL;
  children[cchildren].arguments[subargc-3] = NULL;
  
  
  int PID = fork();
  if(PID > 0){
    children[cchildren].pid=PID;
    cchildren+=1;
  }
  if(PID < 0){
    fprintf(stderr, "Error: fork failed");
    return 1;
  }
  if(PID == 0){
    dup2(in,0);
    dup2(out,1);
    dup2(err,2);
    close(in);
    close(out);
    close(err);
    for(int i=0; i<fdc; i++){
      close(fds[i]);
    }
    
    execvp(subargs[0], subargs);

    close(0);
    close(1);
    close(2);
    
    return 1;
  }
  
  return 0;
}

void catch(int sig){
  fprintf(stderr, "%d caught\n", sig);
  exit(sig);
}

int main(int argc, char* argv[]){
  setvbuf(stdout, NULL, _IONBF, 0);
  int exitstatus = 0;
  int signalstatus = -1;
  int fileDescriptors[200];
  int fdindex = 0;
  
  static struct option args[] = {
				 {"rdonly", 1, NULL, 'r'},
				 {"wronly", 1, NULL, 'w'},
				 {"command", 1, NULL, 'c'},
				 {"verbose", 0, NULL, 'v'},
				 {"append", 0, NULL, 'a'},
				 {"cloexec", 0, NULL, 'b'},
				 {"creat", 0, NULL, 'd'},
				 {"directory", 0, NULL, 'e'},
				 {"dsync", 0, NULL, 'f'},
				 {"excl", 0, NULL, 'g'},
				 {"nofollow", 0, NULL, 'h'},
				 {"nonblock", 0, NULL, 'i'},
				 {"rsync", 0, NULL, 'j'},
				 {"sync", 0, NULL, 'k'},
				 {"trunc", 0, NULL, 'l'},
				 {"rdwr", 1, NULL, 'm'},
				 {"pipe", 0, NULL, 'n'},
				 {"close", 1, NULL, 'o'},
				 {"abort", 0, NULL, 'p'},
				 {"catch", 1, NULL, 'q'},
				 {"ignore", 1, NULL, 's'},
				 {"default", 1, NULL, 't'},
				 {"pause", 0, NULL, 'u'},
				 {"wait", 0, NULL, 'x'},
				 {"chdir", 1, NULL, 'y'},
				 {0,         0,    0,  0}
  };

  int opt, ret;
  int verbose, flags = 0;
  int pipefds[2];
  verbose =0;

  while((opt=getopt_long(argc, argv, "", args, NULL)) != -1) {
    if(opt == '?'){
      fprintf(stderr, "Error: Unrecognized argument\nUsage: ./simpsh --rdonly [file] --wronly [file] --command [stdin] [stdout] [stderr] [cmd] [args] --verbose\n");
      exitstatus = max(exitstatus,1);
    }

    switch(opt){
    case 'r':
      if(verbose)
	printf("--rdonly %s\n", optarg);

      fileDescriptors[fdindex] = open(optarg, O_RDONLY | flags, 0666);
      if(fileDescriptors[fdindex] < 0 ){
	fprintf(stderr, "Error #%d: Error opening file %s\n%s\n",
		errno, optarg, strerror(errno));
	exitstatus = max(exitstatus,1);
      }
      fdindex += 1;
      flags = 0;
      break;
    case 'w':
      if(verbose)
	printf("--wronly %s\n", optarg);
      fileDescriptors[fdindex] = open(optarg, O_WRONLY | flags, 0666);
      if(fileDescriptors[fdindex] < 0 ){
        fprintf(stderr, "Error #%d: Error opening file %s\n%s\n",
                errno, optarg, strerror(errno));
        exitstatus = max(exitstatus,1);
      }
      fdindex += 1;
      flags = 0;
      break;
    case 'c':
      optind -= 1;
      ret = call_command(argc, argv, &optind, fdindex, fileDescriptors,verbose);
      if(ret != 0)
	exitstatus = max(exitstatus,1);
      break;
    case 'v':
      verbose = 1;
      break;
    case 'a':
      if(verbose)
	printf("--append\n");
      flags |= O_APPEND;
      break;
    case 'b':
      if(verbose)
	printf("--cloexec\n");
      flags |= O_CLOEXEC;
      break;
    case 'd':
      if(verbose)
	printf("--creat\n");
      flags |= O_CREAT;
      break;
    case 'e':
      if(verbose)
	printf("--directory\n");
      flags |= O_DIRECTORY;
      break;
    case 'f':
      if(verbose)
	printf("--dsync\n");
      flags |= O_DSYNC;
      break;
    case 'g':
      if(verbose)
	printf("--excl\n");
      flags |= O_EXCL;
      break;
    case 'h':
      if(verbose)
	printf("--nofollow\n");
      flags |= O_NOFOLLOW;
      break;
    case 'i':
      if(verbose)
	printf("--nonblock\n");
      flags |= O_NONBLOCK;
      break;
    case 'j':
      if(verbose)
	printf("--rsync\n");
      flags |= O_RSYNC;
      break;
    case 'k':
      if(verbose)
	printf("--sync\n");
      flags |= O_SYNC;
      break;
    case 'l':
      if(verbose)
	printf("--trunc\n");
      flags |= O_TRUNC;
      break;
    case 'm':
      if(verbose)
        printf("--rdwr %s\n", optarg);
      fileDescriptors[fdindex] = open(optarg, O_RDWR | flags, 0666);
      if(fileDescriptors[fdindex] < 0 ){
        fprintf(stderr, "Error #%d: Error opening file %s\n%s\n",
                errno, optarg, strerror(errno));
        exitstatus = max(exitstatus,1);
      }
      fdindex += 1;
      flags = 0;
      break;
    case 'n':
      if(verbose)
	printf("--pipe\n");
      if(pipe(pipefds) < 0){
	fprintf(stderr, "Error #%d: Pipe error \n%s\n", errno, strerror(errno));
	exitstatus = max(exitstatus, 1);
      }
      fileDescriptors[fdindex] = pipefds[0];
      fdindex += 1;
      fileDescriptors[fdindex] = pipefds[1];
      fdindex += 1;
      break;
    case 'o':
      if(verbose)
	printf("--close %s\n", optarg);
      close(fileDescriptors[atoi(optarg)]);
      fileDescriptors[atoi(optarg)] = -1;
      break;
    case 'p':
      if(verbose)
	printf("--abort\n");
      fflush(stdout);
      char* segf = NULL;
      *segf = 'f';
      break;
    case 'q':
      if(verbose)
	printf("--catch %s\n", optarg);
      signal(atoi(optarg), catch);
      break;
    case 's':
      if(verbose)
        printf("--ignore %s\n", optarg);
      signal(atoi(optarg), SIG_IGN);
      break;
    case 't':
      if(verbose)
        printf("--default %s\n", optarg);
      signal(atoi(optarg), SIG_DFL);
      break;
    case 'u':
      if(verbose)
	printf("--pause\n");
      pause();
      break;
    case 'x':
      if(verbose)
	printf("--wait\n");
      while(1){
	int exitstat, i;
	int pid = waitpid(-1, &exitstat,0);
	if(pid < 0){
	  break;
	}

	for(int k=0; k<cchildren; k++){
	  if(children[k].pid == pid){
	    i=k;
	    break;
	  }
	}


	if(WIFEXITED(exitstat)){
	  printf("exit %d", WEXITSTATUS(exitstat));
	  for(int j=0; children[i].arguments[j] != NULL; j++)
	    printf(" %s", children[i].arguments[j]);
	  printf("\n");
	  fflush(stdout);
	  exitstatus = max(exitstatus, WEXITSTATUS(exitstat));
	}

	if(WIFSIGNALED(exitstat)){
	  printf("signal %d", WTERMSIG(exitstat));
	  for(int j=0; children[i].arguments[j] != NULL; j++)
	    printf(" %s", children[i].arguments[j]);
	  printf("\n");
	  fflush(stdout);
	  signalstatus = max(signalstatus, WTERMSIG(exitstat));
	}
      }
      break;
    case'y':
      if(verbose)
	printf("--chdir %s\n", optarg);
      if(chdir(optarg) < 0){
	fprintf(stderr, "Error: chdir error, cannot change directory to %s\n", optarg);
	exitstatus = max(exitstatus, 1);
	break;
      }
    }
      
  
  }

  if(signalstatus >=0 ){
    signal(signalstatus, SIG_DFL);
    raise(signalstatus);
  }
  exit(exitstatus);

}
