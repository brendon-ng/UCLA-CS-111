//NAME: Brendon Ng
//EMAIL: brendonn8@gmail.com
//ID: 304925492

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

void handle_input(char* file){
  int fd = open(file, O_RDONLY);
  if(fd < 0){
    //Error Opening File
    fprintf(stderr, "Error #%d: Due to '--input' arg\nError Opening file: %s\n%s\n", errno, file, strerror(errno));
    exit(1);
  }
  
  dup2(fd, 0);
  close(fd);
}

void handle_output(char* file) {
  int fd = creat(file, S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
  if(fd < 0){
    fprintf(stderr, "Error #%d: Due to '--output' arg\nError Opening output file: %s\n%s\n", errno, file, strerror(errno));
    exit(2);
  }

  dup2(fd,1);
  close(fd);
}
void catch_func(int sig){
  if(sig == SIGSEGV){
    fprintf(stderr, "Error: Segmentation Fault caused by --segfault. Caught by --catch\n");
    exit(4);
  }
}

void segfault(int catch, int dump){
  if(catch && !dump)
    signal(SIGSEGV, catch_func);
  char* segf = NULL;
  *segf = 'f';
}

int main(int argc, char* argv[])
{
  static struct option args[] = {
				 {"input", 1, NULL, 'i'},
				 {"output", 1, NULL, 'o'},
				 {"segfault", 0, NULL, 's'},
				 {"catch", 0, NULL, 'c'},
				 {"dump-core", 0, NULL, 'd'},
				 {0, 0, 0, 0}
  };

  int opt;
  int catch, segf, dump = 0;
  while((opt = getopt_long(argc, argv, "", args, NULL)) != -1) {
    if(opt =='?'){
      fprintf(stderr, "Error: Unrecognized Argument\nCorrect Usage: ./lab0 --input=[fileName] --output=[fileName] --segfault --catch --dump-core\n");
      exit(3);
    }
    switch (opt){
      case 'i':
	handle_input(optarg);
	break;
      case 'o':
	handle_output(optarg);
	break;
      case 's':
	segf = 1;
	break;
      case 'c':
	catch = 1;
	break;
      case 'd':
	dump = 1;
	break;
    }
  }
  if(segf == 1)
    segfault(catch, dump);
    
  while(1){
    char* buf = (char*) malloc(sizeof(char));

    ssize_t size_read = read(0, buf, 1);
    if(size_read == 0)
      break;
    else if(size_read < 0){
      fprintf(stderr, "Error #%d: Read Input Error\n%s",errno,strerror(errno));
      exit(-1);
    }

    ssize_t written = write(1, buf, 1);
    if(written < 0) {
      fprintf(stderr, "Error #%d: Write to Output Error\n%s",errno,strerror(errno));
      exit(-1);
    }
  }

  exit(0);
}
