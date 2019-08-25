#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include<time.h>


static long sum=0;

void *load(void*);
//threads for each client
pthread_t threads[100000];
//increase array of pthread_t before increasing N
int N=1; 			//defines no of thrads with which to load the client
pthread_mutex_t mutex;


int main() {
	// create a socket
	
	long count;
	// struct timeval tv1,tv2;
	//count=1;

	long input[100000];
	

	//initializing sockid array to -1
	for(long i=0;i<100000;i++)
		input[i]=i;

	//unsigned long time_in_micros1,time_in_micros2;
	// gettimeofday(&tv1,NULL);


	//printf("Creating threads  \n");
	for (count = 0; count < N; count++)
	{
	    //printf("Inside for loop %d\n",count );
		if(pthread_create(&threads[count], NULL , load , (void*) &input[count])<0){
			perror("error");
			return 1;
		}
		//sleep(1);
	}

	//all thread should join before main program exit

	for (count = 0; count < N; count++)
	{
	//	printf("count -> %d\n",count);
		pthread_join(threads[count], NULL);
	}

	//printf("Sum ----->   %ld\n",sum );
	// gettimeofday(&tv2,NULL);
	// while((tv2.tv_sec - tv1.tv_sec) < 30) {
	// 	gettimeofday(&tv2,NULL);	
	// }
	// sleep(1000);

	return 1;
	}



void *load(void *ptr)
{	
// if (pthread_mutex_init(&mutex,NULL)!=0)
// {
// 	/* code */
// 	printf("error mutex\n");
// }

//int accept_socket=*((int *) ptr);
long countInt=(*(long* ) ptr);

//printf("Inside Load ### %ld\n",countInt );

struct timeval tv1,tv2,lstart,lend;
	//when threads enters into the method, starttime for 30sec
	gettimeofday(&tv1,NULL);

long deltaInUSecs=0,lapseInUSecs=0;
long cnt=0;
int temp=100;

	while(deltaInUSecs<300000000){
	/*while(temp>0){
		temp--;*/
			gettimeofday(&lstart,NULL);

	    	gettimeofday(&tv2,NULL);
	deltaInUSecs = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
	//deltaInUSecs = (tv2.tv_sec - tv1.tv_sec)*1000000;
	//printf("Inside while %ld\n",countInt );
	long network_socket;
	
	//printf("Running count %d\n",countInt );
	network_socket=socket(AF_INET, SOCK_STREAM,0);
	//printf("Inside while printing nw socket %d count=%d\n",network_socket,countInt );
	//printf("network_socket=%d\n",network_socket );

	//SPECIFY AN ADDRESS FOR THE SOCKET
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port=htons(9001);
	server_address.sin_addr.s_addr =inet_addr("10.42.0.1");
	// server_address.sin_addr.s_addr =INADDR_ANY;
	
	long connection_status = connect(network_socket,(struct sockaddr *) &server_address ,sizeof(server_address));
	//check for error with the connection
	if (connection_status == -1)
	{
		printf("There was an error making a connection to the remote socket \n\n Start server first \n");
		return ;
	}

	///recieve data from the server
	char server_response[256];
	

	//printing connection establishment acknowledgemnet
	if(recv(network_socket,server_response, sizeof(server_response),0)<0){
		printf(" zhol\n");
	}
	//printf("%s\n", server_response);



	
	//___________________________________________________________________________________
	//MENU
	char clientID[30];
	char password1[30];
	char password2[30];
	int passFlag;			//flag to check comparision between two passwords
	char status[32];

	sprintf(clientID, "%ld", countInt);
	sprintf(password1, "%ld", countInt);
	
	//printf("printing client id and password after conversion %s %s %d\n",clientID,password1,countInt );

			//Registeration

		
			if(send(network_socket,"Registration", sizeof("Registration"),0)<0)
			{
				//telling server that client want to register
				printf("Send of Registration failed\n");
				return 1;

			}

			//printf("Before printing acknowledgement\n");
		
			recv(network_socket,&server_response, sizeof(server_response),0);		//recieving acknowledgement "Inside Registration server"
			//printf("%s\n",server_response );

			//sending user id
			send(network_socket,clientID, sizeof(clientID),0);
			
			//printf("clientID = %s\n",clientID);
			recv(network_socket,&server_response, sizeof(server_response),0);		//recieving acknowledgement "Username Recieve Success"
			//printf("Before Sending password=%s\n", server_response);
			send(network_socket,password1, sizeof(password1),0);					//sending user password

			recv(network_socket,&status, sizeof(status),0);							//recieving status of registration
			//printf("Status:-%s\n",status);

			//pthread_exit(NULL);
			//main(network_socket);
			

			gettimeofday(&lend,NULL);

	lapseInUSecs = (lend.tv_sec - lstart.tv_sec)*1000000+(lend.tv_usec - lstart.tv_usec);
		
		printf("%ld\n",lapseInUSecs );
		//	pthread_mutex_lock(&mutex);
			sum=sum+lapseInUSecs;
		//	pthread_mutex_unlock(&mutex);
			close(network_socket);	
			network_socket=-1;
			countInt+=N;
	cnt++;
	}



return ;

}
	

