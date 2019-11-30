//NAME: Brendon Ng
//EMAIL: brendonn8@gmail.com
//ID: 304925492

#include <stdio.h>
#include <getopt.h> //for getopt_long(3)
#include <stdlib.h> //for atoi
#include <string.h> //for atoi
#include <mraa.h>
#include <poll.h> //for poll
#include <math.h> //for log
#include <sys/time.h> //gettimeofday(2)
#include <time.h> //localtime()
#include <sys/types.h> //open(2)
#include <sys/stat.h> //open(2)
#include <fcntl.h> //open(2)
#include <unistd.h> //write(2)


int period = 1;
char scale = 'F';
int log_bool = 0;
int logFile;
mraa_aio_context temp;
mraa_gpio_context button;

struct timeval now;

void shutdown(){
	struct tm* local = localtime(&now.tv_sec);

	char report[20];
	sprintf(report, "%02d:%02d:%02d SHUTDOWN\n", local->tm_hour, local->tm_min, local->tm_sec);

	fprintf(stdout, report);

	if(log_bool){
		if(write(logFile, report, strlen(report)) < 0){
			fprintf(stderr, "Error writing shutdown to logfile\n");
			mraa_aio_close(temp);
			mraa_gpio_close(button);
			exit(1);
		}
	}

	mraa_aio_close(temp);
	mraa_gpio_close(button);
	exit(0);
}


int main(int argc, char** argv){
	period = 1;
	scale = 'F';
	log_bool = 0;

	static struct option args[] = {
		{"period", 1, NULL, 'p'},
		{"scale", 1, NULL, 's'},
		{"log", 1, NULL, 'l'},
		{0,     0,    0,  0}
	};

	int opt;
	while((opt = getopt_long(argc, argv, "", args, NULL)) != -1){
		if(opt == '?'){
			fprintf(stderr, "Error: Unrecognized argument\n");
			exit(1);
		}

		switch(opt){
			case 'p':
				period = atoi(optarg);
				break;
			case 's':
				for(int i=0; optarg[i] != '\0'; i++){
					if(optarg[i]!='F'&&optarg[i]!='C'){
						fprintf(stderr, "Error: Invalid argument for option --scale.\nUsage: --scale=F or --scale=C\n");
						exit(1);
					}
				}
				scale = optarg[0];
				break;
			case 'l':
				log_bool = 1;
				logFile = open(optarg, O_WRONLY|O_CREAT|O_TRUNC, 0666);
				if(logFile < 0){
					fprintf(stderr, "Error opening log file\n");
					exit(1);
				}
				break;
		}
	}

	//initialize sensors
	temp = mraa_aio_init(1);
	if(temp == NULL){
		fprintf(stderr, "Error initializing temperature sensor\n");
		mraa_deinit();
		exit(1);
	}
	button = mraa_gpio_init(14);
	if(button == NULL){
		fprintf(stderr, "Error initializing button\n");
		mraa_deinit();
		exit(1);
	}

	printf("success");

	if(mraa_gpio_dir(button, MRAA_GPIO_IN) != MRAA_SUCCESS){
		fprintf(stderr, "Error Setting GPIO director for button\n");
		mraa_aio_close(temp);
		mraa_gpio_close(button);
		exit(1);
	}
	
	if(mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &shutdown, NULL) != MRAA_SUCCESS){
		fprintf(stderr, "Error connecting button to shutdown\n");
		mraa_aio_close(temp);
		mraa_gpio_close(button);
		exit(1);
	}




	mraa_aio_close(temp);
	mraa_gpio_close(button);
	exit(0);
}
