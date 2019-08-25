#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>

void *sender(void *ptr);
void *reciever(void *ptr);
//void login();
//void newUser();
void clientToClient(int,char*);
void clientConnect(int);
void clientWait(int , char*);
void  startChat();
void clientInterface(void *ptr);


void menu(int);


int main() {
	// create a socket
	int network_socket;
	
	network_socket=socket(AF_INET, SOCK_STREAM,0);

	//SPECIFY AN ADDRESS FOR THE SOCKET
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port=htons(9001);
	server_address.sin_addr.s_addr =INADDR_ANY;
	
	int connection_status = connect(network_socket,(struct sockaddr *) &server_address ,sizeof(server_address));
	//check for error with the connection
	if (connection_status == -1)
	{
		printf("There was an error making a connection to the remote socket \n\n Start server first \n");
		return;
	}

	///recieve data from the server
	char server_response[256];
	

	//printing connection establishment
	recv(network_socket,&server_response, sizeof(server_response),0);
	printf("%s\n", server_response);



	//calling the menu at clientt side
	menu(network_socket);
	





//Running sender and reciever thread
	/*t1= pthread_create(&threads[0], NULL , sender , (void*) &network_socket);
	t2= pthread_create(&threads[1], NULL , reciever , (void*) &network_socket);	

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

*/
	//close socket
	close(network_socket);
	return 0;
	}



void menu(int network_socket)
{
	//___________________________________________________________________________________
	//MENU
	char clientID[30];
	char password1[30];
	char password2[30];
	int passFlag;			//flag to check comparision between two passwords
	int status[32];


	int choice;
	printf("\n***MENU***\n");
	printf("1.Login\n");
	printf("2.New User\n");
	printf("3.Exit\n");
	printf("----------------\n");
	printf("Enter your Choice:-\n");
	scanf("%d",&choice);

	switch(choice){
		case 1:			//Login

	
			//printf("You are in Login\n");
			send(network_socket,"Login", sizeof("Login"),0);
			
			printf("Enter Client Id\n");
			scanf("%s",clientID);
			send(network_socket,clientID, sizeof(clientID),0);
			
			printf("Enter Password\n");
			scanf("%s",password1);

			send(network_socket,password1, sizeof(password1),0);
			recv(network_socket,&status, sizeof(status),0);
			printf("Staus:-%s\n",status);
			
			if(strcmp(status,"Success")==0)
			{
			//printf("bug inside if success\n");
			clientToClient(network_socket,clientID);
			}
			else
			{
			printf("Authentication Failed. Please Try again\n");
			main(network_socket);
			return;
			}


			break;
		
		case 2:			//Registeration


			send(network_socket,"Registration", sizeof(clientID),0);	//telling server that client want to register
			printf("Enter  Client Id\n");
			scanf("%s",clientID);
			send(network_socket,clientID, sizeof(clientID),0);
			
			printf("Enter Password\n");
			scanf("%s",password1);

			printf("Confirm Password\n");
			scanf("%s",password2);
			passFlag=strcmp(password1,password2);		//compare password1 and password2
			if(passFlag==0)			//returns 0 if both password are same
			{
				printf("Same password.\n");
			}
			else 
			{
				printf("Password does not match.\n Try again.\n");
				main(network_socket);					//this will call menu again so user can try again
				return;
			}
			
			send(network_socket,clientID, sizeof(clientID),0);
			send(network_socket,password1, sizeof(password1),0);

			recv(network_socket,&status, sizeof(status),0);
			printf("Status:-%s\n",status);
			main(network_socket);
			return;

			


			break;
		case 3:				//Exit
			exit(0);
			break;

		default:
			printf("Invalid Option\n Try again\n");
			menu(network_socket);				//this will call menu again so user can try again
			return;
				


	}


}
	

void clientToClient(int network_socket,char* userID)		//after authentication, client to client connection
{
	pthread_t threads[2];
	int t1,t2;
	int choice;
	printf("Client to Client function started.\n");
	

	printf("************MENU***** %s ****************************\n",userID);
	printf("1. To connect Other client \n");
	printf("2. To wait for other client request\n");
	printf("Enter your choice :\t");
	scanf("%d",&choice);

	char msg[40];

	switch(choice){

		case 1:
			//ack the server
			strcpy (msg,"otherPort");
			send(network_socket,msg,sizeof(msg),0);

			//Running wait and connect thread
			//t2= pthread_create(&threads[1], NULL , clientConnect ,(void*) &network_socket);	
			clientConnect(network_socket);
			

			//pthread_join(threads[0], NULL);
			//pthread_join(threads[1], NULL);
	
			break;

		case 2:
			//self socket
			strcpy(msg,"selfPort");
			printf("Waiting for other client to connect\n" );
			send(network_socket,msg,sizeof(msg),0);

			//printf("bug inside case2" );
			//t1= pthread_create(&threads[0], NULL , clientWait , (void*) &network_socket,userID);
			clientWait(network_socket,userID);
			break;
		default:
			printf("Invalid Optin\n Try again. \n");


		
	}


	
	
	
}
		

void clientConnect(int network_socket)
{
	char user[64];
	char port[64];
	pthread_t threads[2];
	int t1,t2;
	//int network_socket=*((int *) ptr);

	//ack server about transaction

	printf("Enter user name of the client to whome you want to connect\n");
	scanf("%s",&user);
	send(network_socket,user, sizeof(user),0);
	//printf("after send \n" );
	recv(network_socket,&port, sizeof(port),0);
	printf("Port recieved is %s\n",port);
	//printf("bug old Socket %d\n",network_socket );

	int p=atoi(port);
	//printf("bug int port %d\n", p);
	close(network_socket);
	/*int true = 1;
	setsockopt(network_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));*/

	int connectSocket;
	
	connectSocket=socket(AF_INET, SOCK_STREAM,0);
	//printf("bug Socket return value %d\n",connectSocket);
	printf("New socket created.\n");
	//SPECIFY AN ADDRESS FOR THE SOCKET
	struct sockaddr_in server_address3;
	server_address3.sin_family=AF_INET;
	server_address3.sin_port=htons(p);
	server_address3.sin_addr.s_addr =INADDR_ANY;
	
	int connection_status = connect(connectSocket,(struct sockaddr *) &server_address3 ,sizeof(server_address3));
	//check for error with the connection
	if (connection_status == -1)
	{
		printf("There was an error making a connection to the remote socket \n\n Start server first \n");
		 perror("Error: ");
		//return;
	}

	///recieve data from the server
	char server_response[256];
	

	//printing connection establishment
	recv(connectSocket,&server_response, sizeof(server_response),0);
	printf("Data Reacieved from client %s\n", server_response);
	t1= pthread_create(&threads[0], NULL , sender , (void*) &connectSocket);
	t2= pthread_create(&threads[1], NULL , reciever , (void*) &connectSocket);	

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);





}

void clientWait(int network_socket , char * userID)
{
	pthread_t threads[2];
	int t1,t2;
	//printf("bug inside client wait\n");

	//int network_socket=*((int *) ptr);

	char port[64];
	//create new socket with self port 
	// request server for port no
	send(network_socket,userID, sizeof(userID),0);
	//printf("bug after send\n");
	recv(network_socket,&port, sizeof(port),0);
	printf("Port recieved is %s\n",port);
	//printf("bug old Socket %d\n",network_socket );
	int p=atoi(port);
	//printf("bug int port %d\n", p);

	//CLOSE the connection with server with port no 9001
	close(network_socket);
	/*int true = 1;
	setsockopt(network_socket,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int));*/

	// create a  server socket
	int newSocket;
	newSocket = socket(AF_INET, SOCK_STREAM,0);

	//define server address
	struct sockaddr_in server_address2;
	server_address2.sin_family=AF_INET;
	server_address2.sin_port=htons(p);
	server_address2.sin_addr.s_addr =INADDR_ANY;


	int s1,s2;
	//bind the socket to our specified IP and port
	s1=bind(newSocket, (struct sockaddr*) &server_address2, sizeof(server_address2));
	//printf("bug bind %d\n",s1 );

	s2=listen(newSocket, 5);
	//printf("bug listen %d\n",s2 );

	
	int accept_socket;
	//printf("bug waitinng at accept\n");
	accept_socket = accept(newSocket, NULL ,NULL);
	//clientInterface(accept_socket);
	//printf("bug After accept\n");
	printf("New socket created.\n");
	send(accept_socket,"Client to client connected", sizeof("Client to client connected"),0);

	t1= pthread_create(&threads[0], NULL , sender , (void*) &accept_socket);
	t2= pthread_create(&threads[1], NULL , reciever , (void*) &accept_socket);	

	pthread_join(threads[0], NULL);
	pthread_join(threads[1], NULL);

	printf("Server socket closed");
	close(newSocket);


}


void clientInterface(void *ptr){
	int network_socket=*((int *) ptr);

	printf("Client connected\n");
	send(network_socket,"connected in client interface", sizeof("connected in client interface"),0);


}



void  *sender( void *ptr)
{
	char chat_message[256];
	int network_socket=*((int *) ptr);
	while(strcmp(chat_message,"bye")!=0)
	{
	printf("\ntype now:-\n");
	gets(chat_message);
	
	//scanf("%s",chat_message);
	send(network_socket,chat_message, sizeof(chat_message),0);
	//send(client_socket,name, sizeof(name),0);
	}

}

void *reciever(void *ptr)
{
	char client_response[256];
	int network_socket=*((int *) ptr);
	while(strcmp(client_response,"bye")!=0)
	{
	 recv(network_socket,&client_response, sizeof(client_response),0);
	 //sleep(2);
	 //print out the server's response
	 if(strcmp(client_response,"")!=0)
	 printf("\nData recieved: %s\n", client_response);
	}
}
