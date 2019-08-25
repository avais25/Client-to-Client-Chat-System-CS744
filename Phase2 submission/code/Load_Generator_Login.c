#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>



void *loadW(void*);
// void *loadC(int);
//threads for each client
pthread_t threads[200000];
int N=100; 			//defines no of thrads with which to load the client



int main() {
	// create a socket
	
	long count;
	//count=1;

	long input[100000];
	

	//initializing sockid array to -1
	for(long i=0;i<100000;i++)
		input[i]=i;


	//printf("Creating threads  \n");
	for (count =0 ; count < N; count++)
	{
		//printf("Inside for loop %d\n",count );


		
		if(pthread_create(&threads[count], NULL , loadW , (void*) &input[count])<0){
			perror("error");
			return 1;
		}
	}

	for (int count = 0; count < N; count++)
	{
		pthread_join(threads[count], NULL);
	}

	return 1;

}



void *loadW(void *ptr)
{
	long countInt=(*(long* ) ptr);


	struct timeval tv1,tv2,lstart,lend;
	//when threads enters into the method, starttime for 30sec
	gettimeofday(&tv1,NULL);

	long deltaInUSecs=0,lapseInUSecs=0;
	long cnt=0;


	



	while(deltaInUSecs<300000000){
	gettimeofday(&lstart,NULL);

	gettimeofday(&tv2,NULL);
	deltaInUSecs = (tv2.tv_sec - tv1.tv_sec)*1000000 + (tv2.tv_usec - tv1.tv_usec);
	//deltaInUSecs = (tv2.tv_sec - tv1.tv_sec)*1000000;
	//printf("Inside while %ld\n",countInt );
	long network_socket;
	
	//printf("Running count %d\n",countInt );
	network_socket=socket(AF_INET, SOCK_STREAM,0);


	//int network_socket;
	

	//printf("Running count from creation of thread %d\n",countInt );
	//printf("Running count from while %d\n",cnt );
	//network_socket=socket(AF_INET, SOCK_STREAM,0);
	//rintf("network_socket=%d\n",network_socket );

	//SPECIFY AN ADDRESS FOR THE SOCKET
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port=htons(9001);
	//server_address.sin_addr.s_addr =INADDR_ANY;
	server_address.sin_addr.s_addr =inet_addr("10.15.26.165");
	
	int connection_status = connect(network_socket,(struct sockaddr *) &server_address ,sizeof(server_address));
	//check for error with the connection
	if (connection_status == -1)
	{
		printf("There was an error making a connection to the remote socket \n\n Start server first \n");
		return 1;
	}

	///recieve data from the server
	char server_response[256];
	

	//menu() function started-----------------

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
	int status[64];

	sprintf(clientID, "%d", countInt);
	sprintf(password1, "%d", countInt);
	

			//Registeration

			
			if(send(network_socket,"Login", sizeof("Login"),0)<0)
			{
				//telling server that client want to register
				printf("zhl2\n");
				return;

			}


			//loginClient() started------------------
			
			recv(network_socket,&server_response, sizeof(server_response),0);		//recieving acknowledgement "Inside login server"
			//printf("%s\n",server_response );
			send(network_socket,clientID, sizeof(clientID),0);
			
			
			recv(network_socket,&server_response, sizeof(server_response),0);		//recieving acknowledgement from server
			send(network_socket,password1, sizeof(password1),0);					//sending user password

			recv(network_socket,&status, sizeof(status),0);							//recieving status from the server ;; Success;; Invalid username ;; Wrong Password

			//printf("Status:-%s\n",status);

			if(strcmp(status,"Success")!=0)										//if status is "Success"
			{
			close(network_socket);	
			network_socket=-1;
			countInt+=10;
			cnt++;
			continue;
			}



			//clientToClient() function started-----------------


	char port[64];
//	printf("Client to Client function started.\n");



	send(network_socket,"Success", sizeof("Success"),0);		//sending acknowledgement to the server
	
	recv(network_socket,&port, sizeof(port),0);			//recieving its own port



	char msg[40];
	strcpy(msg,"selfPort");
	//printf("Waiting for other client to connect\n" );
	send(network_socket,msg,sizeof(msg),0);								//sending the messege "selfPort" to the server

	close(network_socket);	
	network_socket=-1;


//clientWait function started ---------------------------------------------------------------------------------
	
	gettimeofday(&lend,NULL);

	lapseInUSecs = (lend.tv_sec - lstart.tv_sec)*1000000+(lend.tv_usec - lstart.tv_usec);
		
	printf("%ld,",lapseInUSecs );
	
	
	countInt+=10;
	cnt++;

			
}//end of while loop


return;


}
	

