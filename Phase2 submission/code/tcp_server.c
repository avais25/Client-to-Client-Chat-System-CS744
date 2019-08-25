#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <pthread.h>
#include <netinet/in.h>

#include <unistd.h>


#define NTHREAD 400000			//used as size of newClientThread array
#define NSOCKID 400000			//used as size of sockid array
#define LISTEN 400000			//USED IN listen function



//global variable for connection 
//we can put ip address instead of 'localhost'
 static char *host= "localhost";
 static char *user="root";
 static char *pass="synergy";
 static char *dbname="synergy";

//port no of mysql database is 3306
unsigned int port=3306;

//to specify connection time
static char *unix_socket=NULL;

//to show connection like odbc
unsigned int flag=0;


//threads for each client  //not used
//pthread_t threads[100];

//implementing lock
pthread_mutex_t mutex;



//function initialization

void socket_creator(long ptr);
void *clientInterface();
void loginServer(long);
void regServer(long);
void queryPort(int);
void queryPortWait(int);

void clientToClientServer(int,char*);

void queryPortConnect(int,char*);

void *serverExit(void *ptr);


//static long count=0;
int main() {
	
	//initializing mutex  lock
/*	 if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        printf("\nLock initialization failed\n");
        return;
    }*/



	
	long s_port=9001;
	
	
	socket_creator(s_port);
	
	return 0;
	
	}


void socket_creator(long s_port)
	{
	
	pthread_t thread1;			//thread to properly close(exit) server
	int t1;



	// create a  server socket
	long server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM,0);

	//define server address
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port=htons(s_port);
	server_address.sin_addr.s_addr =INADDR_ANY;

	//bind the socket to our specified IP and port
	if(bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address))<0){
		printf("Bind failed at server\n");
		return ;
	}

	listen(server_socket, LISTEN);
		

	t1= pthread_create(&thread1, NULL , serverExit , (void*) &server_socket);				//thread to properly close server

	printf("Waiting for a client to connect.......\n");
	long accept_socket;
	struct sockaddr_in clientAcc;
	//while(accept_socket = accept(server_socket, NULL ,NULL)){

	long sockid[NSOCKID];
	

	//initializing sockid array to -1
	for(long i=0;i<NSOCKID;i++)
		sockid[i]=-1;

	long counter=0;
	long c=sizeof(struct sockaddr_in);

	pthread_t newClientThread[NTHREAD];



	while(1)
	{
		//printf("Accepting a client... XXXX \n");
		sockid[counter] =accept(server_socket, (struct sockaddr *)&clientAcc ,(socklen_t*)&c);
		printf("Counter #XX %ld\n",sockid[counter]);
		//pthread_create(&threads[count], NULL , clientInterface , (void*) &accept_socket);
		if(pthread_create(&newClientThread[counter], NULL , clientInterface , (void*) &sockid[counter])<0){
			perror("Failed to create thread.. Exiting..");
			return 1;
		}

		//this count is used in the folowing printf
		counter++;		
		printf("Client #%ld connected\n",counter );

		//finding the free location in the sockid array
		// while(sockid[counter]!=-1)
		// {
		// 	  counter=(counter+1)%NSOCKID;
		// }

	}
	if (accept_socket<0)
	{
		printf("No socket accepted\n");
	}



	printf("Server socket closed");
	close(server_socket);

	return;
	
	
	
	}



void *clientInterface(void *ptr){ 		//this function send the init messege to  client and handles menu() function of the client
	printf("A client interface started\n");

	
	


	
	long accept_socket=*((long *) ptr);
	char client_response[256];		//to store response from the server
	char init_message[256] = "You are now connected.";
	
	

	//printf("bug before send \n");
	//send initialization the message
	send(accept_socket,init_message, sizeof(init_message),0);
	printf("Initialization message sent\n");



	//Getting client selection from the menu
	recv(accept_socket,&client_response, sizeof(client_response),0);			//recieving menu response from the client , either client wants to Login or Register


	printf("Recieved a response from client . Response= %s\n",client_response);		//printing menu response




	//Running loginServer or regServer
	if(strcmp(client_response,"Login")==0)			//if response was login
	{
	printf("Client wants to do %s\n",client_response);			//printing menu response again
	loginServer(accept_socket);
	}


	if(strcmp(client_response,"Registration")==0)			//if response was registeration
	{
		printf("Client wants to do %s\n",client_response);			//printing menu response again
		regServer(accept_socket);				//this function maps with regClient in the client code, it implements Registeration
	}

	//The sever should close the accept socket and change the accept counter and exit the thread
	if(strcmp(client_response,"Exit")==0)
	{
		//code need to be added here and in the client
	}	
	


}


void loginServer(long accept_socket)				//this function maps with loginClient()  function in the client code
{
	//char server_response[256];
	char userId[256];
	char upassword[256];
	char client_response[256];
	
	int queryFlag=0;		//query flag 0=invalid uid ;;; 1=Success ;;; 2=Wrong password



	//pthread_t threads[2];
	int t1,t2;				


	send(accept_socket,"Inside login server", sizeof("Inside login server"),0);				//sending acknowledgement "Inside login server" to the client

	
	recv(accept_socket,&userId, sizeof(userId),0);			//recieving user name  from client


	//sending acknowledgement to the client
	send(accept_socket,"Username recieved", sizeof("Username recieved"),0);

	recv(accept_socket,&upassword, sizeof(upassword),0);	//recieving  password from client
	//printf("password%s\n",upassword );
	printf("Recieved id=%s\n",userId);				//Printing recieved user name

	
	//locking 
	if(pthread_mutex_lock(&mutex)!=0)
	{
		perror("Lock Failed\n");
	}


	// to connect db server
	MYSQL *connection;
	// intialize the mysql structure 
	connection=mysql_init(NULL);

	MYSQL_RES *res;				//to store result of the query
	MYSQL_ROW row;				//store the row of  the result



	//to connect database 
	//return 0 on sucessful connection
	if(!(mysql_real_connect(connection,host,user,pass,dbname,port,unix_socket,flag)))
	{
	fprintf(stderr, "\n Error:%s[%d]\n",mysql_error(connection),mysql_errno(connection));
	}

	printf("sql connection sucessful!!\n");
	mysql_query(connection,"select * from clientData");

	printf("query successful\n");

	res=mysql_store_result(connection);

	mysql_close(connection);				//cloasing the mysql connection	
	pthread_mutex_unlock(&mutex);
	//unlocking 
	

	//fetching the user name from obtained result
	while(row=mysql_fetch_row(res))
	{
	if(strcmp(userId,row[1])==0)					//comparing weather userId match
	{
	
		if(strcmp(upassword,row[2])==0)						//comparing weather the password match							
		{
		
		queryFlag=1;								//successful matching of user id and password
		
		}
		else
		{
		
		queryFlag=2;								//wrong password
		}
	}
	
	}

	mysql_free_result(res);					//freeing the result of sql query
	
	

	switch(queryFlag)			//query flag 0=invalid uid ;;; 1=Success ;;; 2=Wrong password
	{
	case 0:			//for error "Invalid Username"

	send(accept_socket,"Invalid username", sizeof("Invalid username"),0);				//sending the error "Invalid Username" to the client
	printf("Invalid Username\n");
	//main();																				//restrting main because of error
	break;




	case 1:				//for success

	send(accept_socket,"Success", sizeof("Success"),0);									//sending the success information "Success" to the client
	printf("Success Login\n");
	


	clientToClientServer(accept_socket,userId);						//this function maps with clientToClient() in the client  code


	break;



	case 2:				//for error "Wrong Password"

	send(accept_socket,"Wrong Password", sizeof("Wrong Password"),0);						//sending the error "Wrong Password" to the client
	printf("Wrong Password\n");
	
	//for implementation of load generator

	//main();																					//restarting main due to "Wrong Password"
	break;
	}

	
	//only for partial impementation of of load generator ;; should be removed
	close(accept_socket);
	accept_socket=-1;
	return;

	

		 
}



void clientToClientServer(int accept_socket,char* userId)							//this function maps with clientToClient() in the client  code
{
	char ack[128];
	char client_response[256];


	char uport[64];
	//get client ID
	recv(accept_socket,&ack, sizeof(ack),0);			//recieving acknowledgement form the client
	printf("Self Username:-  %s\n",userId);										//printing its own username


	//locking 
	if(pthread_mutex_lock(&mutex)!=0)
	{
		perror("Lock Failed\n");
	}
	// to connect db server
	MYSQL *nconnection;
	// intialize the mysql structure 
	nconnection=mysql_init(NULL);

	MYSQL_RES *res;
	MYSQL_ROW row;

	//to coonect database
	//return 0 on sucessful connection
	if(!(mysql_real_connect(nconnection,host,user,pass,dbname,port,unix_socket,flag)))
	{
	fprintf(stderr, "\n Error:%s[%d]\n",mysql_error(nconnection),mysql_errno(nconnection));
	}

	printf("sql connection sucessful!!\n");
	mysql_query(nconnection,"select * from clientData");						//executing my sql query

	printf("query successful\n");

	res=mysql_store_result(nconnection);

	//closing my sql connection
	mysql_close(nconnection);

	//unlocking 
	pthread_mutex_unlock(&mutex);
	

	while(row=mysql_fetch_row(res))
	{
	if(strcmp(userId,row[1])==0)							//comparing usre name to find the port number
	{
	//strcpy(row[0],port);
	printf("Sending logged in client port %s\n",row[0]);					//printing the port no
	send(accept_socket,row[0], sizeof(row[0]),0);			//sending the port no to the client
	
	}
	}

	
	//freeing up space  used by res
	mysql_free_result(res);		
	

	






	// ack from client 
	recv(accept_socket,&client_response, sizeof(client_response),0);				//recieving from clientToClient() in client code ;; it will either be selfPort or otherPort




	printf("Client wants %s\n",client_response );									//printing the response

	if (strcmp(client_response, "selfPort") == 0)									//queryPortWait() function will start if client want selfPort
	{
		queryPortWait(accept_socket);
	}

	if (strcmp(client_response, "otherPort") == 0)									//queryPortConnect() function will start if client want selfPort
	{
		queryPortConnect(accept_socket,userId);
	}


}



void queryPortConnect(int accept_socket,char* cuser)										//this function maps with clientConnect() in hte client code 
{
	printf("Query Port (connect) function started.\n");
	char userId[64];
	char uport[64];
	char client_response[256];

	


	

	send(accept_socket,"Inside Client Connect", sizeof("Inside Client Connect"),0);			//sending acknowledgemt messege


	//get client ID
	recv(accept_socket,&userId, sizeof(userId),0);
	printf("Recieved Username:-  %s\n",userId);					//printing the name of client id  recieved

	

	//locking 
	if(pthread_mutex_lock(&mutex)!=0)
	{
		perror("Lock Failed\n");
	}
	// to connect db server
	MYSQL *nconnection;
	// intialize the mysql structure 
	nconnection=mysql_init(NULL);

	MYSQL_RES *res1,*res2;
	MYSQL_ROW row1,row2;



	//to coonect database
	//return 0 on sucessful connection
	if(!(mysql_real_connect(nconnection,host,user,pass,dbname,port,unix_socket,flag)))
	{
	fprintf(stderr, "\n Error:%s[%d]\n",mysql_error(nconnection),mysql_errno(nconnection));
	}

	printf("sql connection sucessful!!\n");
	



	mysql_query(nconnection,"select * from clientData");			//running my sql query

	printf("query successful\n");


	res2=mysql_store_result(nconnection);
	

	//closing my sql connection
	mysql_close(nconnection);

	//unlocking 
	pthread_mutex_unlock(&mutex);
	
	while(row2=mysql_fetch_row(res2))
	{
		//printf("bug inside while\n");
	if(strcmp(userId,row2[1])==0)							//fetching the userId in mysql result
		{
			//strcpy(row[0],port);
			printf("Sending port of the client to whome he want to connect %s\n",row2[0]);
			send(accept_socket,row2[0], sizeof(row2[0]),0);
			
		}
	}
	printf("Exiting Query Port (connect) function .\n");

	//freeing up accept_socket
	close(accept_socket);
	accept_socket=-1;
	
	//closing mysql connection	
	//mysql_free_result(res1);
	mysql_free_result(res2);
	
}

void queryPortWait(int accept_socket)			//this function maps with clientWait() function in the client code
{
	printf("Query Port (Wait) function started.\n");
	char ack[64];
	//char uport[64];
	//char client_response[256];

	//int accept_socket=*((int *) ptr);
	//freeing up accept_socket
	close(accept_socket);
	accept_socket=-1;


	// //recieving acknowledgement from client
	// recv(accept_socket,&ack, sizeof(ack),0);
	// printf("acknowledgemt from client:-%s\n",ack );
	

	/*//get client ID
	recv(accept_socket,&userId, sizeof(userId),0);
	printf("Recieved Username:-  %s\n",userId);

	// to connect db server
	MYSQL *nconnection;
	// intialize the mysql structure 
	nconnection=mysql_init(NULL);

	MYSQL_RES *res;
	MYSQL_ROW row;



	//to coonect database
	//return 0 on sucessful connection
	if(!(mysql_real_connect(nconnection,host,user,pass,dbname,port,unix_socket,flag)))
	{
	fprintf(stderr, "\n Error:%s[%d]\n",mysql_error(nconnection),mysql_errno(nconnection));
	}

	printf("sql connection sucessful!!\n");
	mysql_query(nconnection,"select * from clientData");

	printf("query successful\n");

	res=mysql_store_result(nconnection);

	//closing mysql connection
	mysql_close(nconnection);

	while(row=mysql_fetch_row(res))
	{
	if(strcmp(userId,row[1])==0)
	{
	//strcpy(row[0],port);
	printf("Sending new port%s\n",row[0]);
	send(accept_socket,row[0], sizeof(row[0]),0);
	
	}
	}

	
	//closing mysql connection	
	mysql_free_result(res);*/
	
}



void regServer(long accept_socket)			//this function maps with regClient in the client code, it implements Registeration
{
	//storing query
	char qry[512];
	char userId[256];
	char upassword[256];

	//int accept_socket=*((int *) ptr);
	printf("Started regServer function\n");				//Printing current running point of the code

	send(accept_socket,"Inside Registration server", sizeof("Inside Registration server"),0);		//sending acknowledgement

	recv(accept_socket,&userId, sizeof(userId),0);							//recieving user id

	printf("Recieved Username= %s \n",userId);		//printing the  recieved  user id

	send(accept_socket,"Username Recieve Success", sizeof("Username Recieve Success"),0);			//sending acknowledgement

	recv(accept_socket,&upassword, sizeof(upassword),0); 		//Recieving  password from client



	

	
	//locking 
	/*if(pthread_mutex_lock(&mutex)!=0)
	{
		perror("Lock Failed\n");
	}*/


	// to connect db server
	MYSQL *connection;
	// intialize the mysql structure 
	connection=mysql_init(NULL);

	MYSQL_RES *res;
	MYSQL_ROW row;



	//to coonect database
	//return 0 on sucessful connection
	if(!(mysql_real_connect(connection,host,user,pass,dbname,port,unix_socket,flag)))
	{
	fprintf(stderr, "\n Error:%s[%d]\n",mysql_error(connection),mysql_errno(connection));
	}
	else
	{
		printf("sql connection sucessful!!\n");

	}
	


	//checking if user name  already exist
	mysql_query(connection,"select * from clientData");
	
	res=mysql_store_result(connection);
	while(row=mysql_fetch_row(res))
	{
	if(strcmp(userId,row[1])==0)
	{

			//closing mysql connection
			mysql_free_result(res);
			mysql_close(connection);


			//unlocking
			//pthread_mutex_unlock(&mutex);
			
			

			printf("Username already exist.\n");
			send(accept_socket,"Username already exist", sizeof("Username already exist"),0);

			//freeing up accept socket file descriptor
			close(accept_socket);
			accept_socket=-1;

		//main();				//main commented for implementation of load generator
		return;
	}
	}

	//inserting user name to the table
	//creating query 
	snprintf(qry, 512, "insert into clientData(userid,password,running) values('%s','%s','0');", userId,upassword);			//running mysql query
	//executing query
	mysql_query(connection,qry);

	
	//res=mysql_store_result(connection);			//no need of res here


	//closing mysql connection
	mysql_free_result(res);
	mysql_close(connection);

	//unlocking
	//pthread_mutex_unlock(&mutex);


	printf("Client Registeration successful\n");
	send(accept_socket,"Client Registeration successful", sizeof("Client Registeration successful"),0);

	close(accept_socket);
	accept_socket=-1;

	//commented to properly implement load generator
	//main();						//starting main menu again
	
}





void *serverExit(void *ptr)
{
	char *ip[16];
	int server_socket=*((int *) ptr);
	int iSetOption = 1;
	printf("Type \"exit\" to close server\n");
	while(1)
	{
		scanf("%s",ip);
		if(strcmp(ip,"exit")==0)
		{
			
			setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&iSetOption,sizeof(iSetOption));			//freeing up binded port
			
			//destroying the mutex lock
			pthread_mutex_destroy(&mutex);


			close(server_socket);			//closing the server socket
			server_socket=-1;
			exit(0);			//exiting from program

		}
		else
		{
			printf("Invalid Option\n");				//if something other than "exit" is input
		}

		
	}
	
}
