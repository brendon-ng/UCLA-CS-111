//NAME: Brendon Ng
//EMAIL: brendonn8@gmail.com
//ID: 304925492

#ifdef DUMMY
int dum = 0;
#define TEMP_PIN 1
#define MRAA_SUCCESS 0
typedef int* mraa_aio_context;
int mraa_aio_read(mraa_aio_context c){
  return 650;
}
int* mraa_aio_init(int c){
  return &dum;
}
void mraa_deinit(){}


#else
#include <mraa.h>
#include <mraa/aio.h>
#define TEMP_PIN 1
#endif

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h> 
#include <math.h> 
#include <sys/time.h> 
#include <time.h>
#include <fcntl.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <netdb.h>

int period = 1;
char scale = 'F';
int log_bool = 0;
int logFile;
int started = 1;
mraa_aio_context temp;
int sock;

char* id;
char* host;
int port;

float getTemp(){
	const int B = 4275;
	const int R0 = 100000;
	float reading = mraa_aio_read(temp);
	float R = 1023.0/reading - 1.0;
	R = R0*R;
	float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
	if(scale == 'F')
		return (temperature * 9.0/5.0) + 32.0;
	else if (scale == 'C')
		return temperature;
	else{
		fprintf(stderr, "Invalid scale\n");
		exit(1);
	}
}

void shut_down(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	struct tm* local = localtime(&tv.tv_sec);

	char report[20];
	sprintf(report, "%02d:%02d:%02d SHUTDOWN\n", local->tm_hour, local->tm_min, local->tm_sec);

	//fprintf(stdout, report);

	if(log_bool){
		if(write(logFile, report, strlen(report)) < 0){
			fprintf(stderr, "Error writing shutdown to logfile\n");
			exit(2);
		}
	}

	if(write(sock, report, strlen(report)) < 0){
		fprintf(stderr, "Error writing shutdown to socket\n");
		exit(2);
	}

	exit(0);
}


void perform_command(char* command){
	if(strcmp(command, "SCALE=F") == 0){
		scale = 'F';
	}
	else if(strcmp(command, "SCALE=C") == 0)
		scale = 'C';
	else if(strncmp(command, "PERIOD=", 7) == 0)
		period = atoi(command+7);
	else if(strcmp(command, "START") == 0)
		started = 1;
	else if(strcmp(command, "STOP") == 0)
		started = 0;
	else if(strcmp(command, "OFF") == 0)
		shut_down();
}



int main(int argc, char** argv){
	period = 1;
	scale = 'F';
	log_bool = 0;

	static struct option args[] = {
		{"period", 1, NULL, 'p'},
		{"scale", 1, NULL, 's'},
		{"log", 1, NULL, 'l'},
		{"id", 1, NULL, 'i'},
		{"host", 1, NULL, 'h'},
		{0,     0,    0,  0}
	};

	int id_present, host_present = 0;
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
			case 'i':
				id = optarg;
				id_present = 1;
				if(strlen(id) != 9){
					fprintf(stderr, "Error: ID must be length 9!\n");
					exit(1);
				}
				break;
			case 'h':
				host = optarg;
				host_present = 1;
				break;
		}
	}

	if(optind==argc || !id_present || !host_present || !log_bool){
		fprintf(stderr, "Error: Missing mandatory parameter!\nMandatory Parameters: id, host, log, a port number\n");
		exit(1);
	}

	port = atoi(argv[optind]);
	if(port <= 1024){
		fprintf(stderr, "Error: Invalid port number! Must be > 1024\n");
		exit(1);
	}

	//Socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0){
		fprintf(stderr, "Error creating socket\n");
		exit(2);
	}

	struct hostent* server = gethostbyname(host);
	if(server == NULL){
		fprintf(stderr, "Error: Could not find server %s\n", host);
		exit(1);
	}
	
	struct sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(struct sockaddr_in));
	server_addr.sin_family = AF_INET;
	memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
	server_addr.sin_port = htons(port);

	if(connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		fprintf(stderr, "Error: Could not connect to server\n");
		exit(2);
	}

	char id_str[15];
	sprintf(id_str, "ID=%s\n", id);
	if(write(logFile, id_str, strlen(id_str)) < 0){
			fprintf(stderr, "Error: Could not write ID to logfile\n");
			exit(2);
	}
	if(write(sock, id_str, strlen(id_str)) < 0){
			fprintf(stderr, "Error: Could not write ID to socket\n");
			exit(2);
	}




	//initialize sensors
	temp = mraa_aio_init(TEMP_PIN);
	if(temp == NULL){
		fprintf(stderr, "Error initializing temperature sensor\n");
		mraa_deinit();
		exit(2);
	}
	
	char* command = malloc(sizeof(char));
	int com_size = 0;

	struct timeval last, cur;
	gettimeofday(&last, NULL);

	while(1){
		struct pollfd fds[1];
		fds[0].fd = sock;
		fds[0].events = POLLIN;

		int poll_stat = poll(fds, 1, 0);
		if(poll_stat == -1){
			fprintf(stderr, "Error with poll\n");
			exit(2);
		}
		if(poll_stat > 0){
			char buf[1];
			if(read(sock, buf, 1) < 0){
				fprintf(stderr, "Error reading stdin\n");
				exit(2);
			}

			if(*buf == '\n'){
				command[com_size] = '\0';
				if(log_bool){
					write(logFile, command,strlen(command));
					write(logFile, "\n", 1);
				}
				//write(sock, command, strlen(command));
				//write(sock, "\n", 1);
				perform_command(command);
				free(command);
				command = malloc(sizeof(char));
				com_size = 0;
			}
			else{
				command[com_size] = *buf;
				com_size += 1;
				command = realloc(command, (com_size+1)*sizeof(char));
			}
		}

		gettimeofday(&cur, NULL);

		if((cur.tv_sec >= last.tv_sec + period) && started){
			float temp = getTemp();
			struct tm* local = localtime(&cur.tv_sec);
			char report[15];
			sprintf(report, "%02d:%02d:%02d %.1f\n", local->tm_hour, local->tm_min, local->tm_sec, temp);
			if(log_bool){
				if(write(logFile, report, strlen(report))<0){
					fprintf(stderr, "Error writing temp report\n");
					exit(2);
				}
				if(write(sock, report, strlen(report))<0){
					fprintf(stderr, "Error writing temp report to socket\n");
					exit(2);
				}
			}
			gettimeofday(&last, NULL);
		}

	}

	exit(0);
}
