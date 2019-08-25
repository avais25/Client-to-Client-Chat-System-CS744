#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <pthread.h>
#include <netinet/in.h>

#include <unistd.h>


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


//threads for each client
pthread_t threads[100];

//function initialization
void *sender(void *ptr);
void *reciever(void *ptr);
void socket_creator(int *ptr);
void *clientInterface();
void loginServer(int);
void regServer(int);
void queryPort(int);



static int count=0;
int main() {
	pthread_t thread1;
	int t1;
	int s_port=9001;
	
	/*t1= pthread_create(&thread1, NULL , socket_creator , (void*) &s_port);
	pthread_join(thread1, NULL);*/
	socket_creator(s_port);
	
	return 0;
	
	}


void socket_creator(int *s_port)
	{
	
	
	//int s_port=*((int *) ptr);



	// create a  server socket
	int server_socket;
	server_socket = socket(AF_INET, SOCK_STREAM,0);

	//define server address
	struct sockaddr_in server_address;
	server_address.sin_family=AF_INET;
	server_address.sin_port=htons(s_port);
	server_address.sin_addr.s_addr =INADDR_ANY;

	//bind the socket to our specified IP and port
	bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

	listen(server_socket, 5);
	

	printf("Waiting for a client to connect.......\n");
	int accept_socket;
	while(accept_socket = accept(server_socket, NULL ,NULL)){

		pthread_create(&threads[count], NULL , clientInterface , (void*) &accept_socket);
		count++;
		printf("Client #%d connected\n",count );
		

	}
	printf("Server socket closed");
	close(server_socket);
	
	//pthread_join(threads[2], NULL);
	
	}



void *clientInterface(void *ptr){ 		//this function send the init messege to client and start the sender and recievr thread 
	printf("A client interface started\n");
	pthread_t interfaceThread[3];
	int accept_socket=*((int *) ptr);
	char client_response[256];		//to store response from the server
	char init_message[256] = "You are now connected.";
//send initialization the message

	//printf("bug before send \n");
	send(accept_socket,init_message, sizeof(init_message),0);
	printf("Initialization message sent\n");
/*//Running sender and reciever thread
	 pthread_create(&interfaceThread[0], NULL , sender , (void*) &accept_socket);
	 pthread_create(&interfaceThread[1], NULL , reciever , (void*) &accept_socket);	

	pthread_join(interfaceThread[0], NULL);
	pthread_join(interfaceThread[1], NULL); */

	//printf("bug sr\n");
	//Getting client selection from the menu
	recv(accept_socket,&client_response, sizeof(client_response),0);
	printf("Recieved a response from client . Response= %s\n",client_response);
	//Running loginServer or regServer
	if(strcmp(client_response,"Login")==0)
	{
	printf("Client wants to do %s\n",client_response);
	loginServer(accept_socket);
	}
	if(strcmp(client_response,"Registration")==0)
	{
		printf("Client wants to do %s\n",client_response);
		regServer(accept_socket);
	}
	


}


void loginServer(int accept_socket)
{
	//char server_response[256];
	char userId[256];
	char upassword[256];
	//int accept_socket=*((int *) ptr);
	int queryFlag=0;		//query flag 0=invalid uid ;;; 1=Success ;;; 2=Wrong password

	//pthread_t threads[2];
	int t1,t2;
	
	recv(accept_socket,&userId, sizeof(userId),0);			//recieving user name and password from client
	recv(accept_socket,&upassword, sizeof(upassword),0);
	
	printf("Recieved id=%s",userId);

	
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

	printf("sql connection sucessful!!\n");
	mysql_query(connection,"select * from clientData");

	printf("query successful\n");

	res=mysql_store_result(connection);

	while(row=mysql_fetch_row(res))
	{
	if(strcmp(userId,row[1])==0)
	{
	//printf("bug %s  %s\n",row[1],row[2]);
		if(strcmp(upassword,row[2])==0)
		{
		//printf("Client Login Successful.\n");
		queryFlag=1;
		//send(accept_socket,"Success", sizeof("Success"),0);
		}
		else
		{
		//printf("bug %s-%s",row[2],upassword);
		//printf("Wrong Password.\n");	//if username match and password did not match
		//send(accept_socket,"Wrong Password", sizeof("Wrong Password"),0);
		queryFlag=2;
		}
	}
	//else
	//printf("Invalid username\n");	//if username did not match
	//send(accept_socket,"Invalid username", sizeof("Invalid username"),0);
	}
	
	switch(queryFlag)
	{
	case 0:
	send(accept_socket,"Invalid username", sizeof("Invalid username"),0);
	main();
	break;

	case 1:
	send(accept_socket,"Success", sizeof("Success"),0);

	//mysql_free_result(res);
	//mysql_close(connection);
	//query port

	queryPort(accept_socket);

	//t1= pthread_create(&threads[0], NULL , queryPort , (void*) &accept_socket);
	//t2= pthread_create(&threads[1], NULL , queryPort , (void*) &accept_socket);	
	//printf("Query port thread started.\n");
	//pthread_join(threads[0], NULL);
	//pthread_join(threads[1], NULL);
	break;

	case 2:
	send(accept_socket,"Wrong Password", sizeof("Wrong Password"),0);
	main();
	break;
	}

	mysql_free_result(res);
	mysql_close(connection);

	//printf("%s",pass);
	//mysql_query(connection,"select * from clientData");
	 
}

void queryPort(int accept_socket)
{
	printf("Query Port function started.\n");
	char userId[64];
	char uport[64];
	char client_response[256];

	//int accept_socket=*((int *) ptr);


	// ack from client 
	recv(accept_socket,&client_response, sizeof(client_response),0);
	printf("Client wants %s\n",client_response );

	//get client ID
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
	mysql_free_result(res);
	mysql_close(nconnection);
}


void regServer(int accept_socket)
{
	//storing query
	char qry[512];
	char userId[256];
	char upassword[256];

	//int accept_socket=*((int *) ptr);


	recv(accept_socket,&userId, sizeof(userId),0);

	recv(accept_socket,&upassword, sizeof(upassword),0); 		//Recieving user name and  password from client

	printf("Recieved Username= %s \n",userId);

	
	
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

	printf("sql connection sucessful!!\n");


	//checking if user name  already exist
	mysql_query(connection,"select * from clientData");
	
	res=mysql_store_result(connection);
	while(row=mysql_fetch_row(res))
	{
	if(strcmp(userId,row[1])==0)
	{
		printf("Username already exist.\n");
		send(accept_socket,"Username already exist", sizeof("Username already exist"),0);
		main();
		return;
	}
	}

	//inserting user name to the table
	snprintf(qry, 512, "insert into clientData(userid,password,running) values('%s','%s','0');", userId,upassword);
	mysql_query(connection,qry);

	printf("Client Registeration successful\n");
	send(accept_socket,"Client Registeration successful", sizeof("Client Registeration successful"),0);
	res=mysql_store_result(connection);

	mysql_free_result(res);
	mysql_close(connection);
	main();
	//printf("%s",pass);
	//mysql_query(connection,"select * from clientData");
}




/*void  *sender( void *ptr)
	{
	char chat_message[256];
	int accept_socket=*((int *) ptr);
	if(strcmp(chat_message,"bye")!=0)
	{
	printf("\ntype now:-\n");
	gets(chat_message);
	
	//scanf("%s",chat_message);
	send(accept_socket,chat_message, sizeof(chat_message),0);
	//send(client_socket,name, sizeof(name),0);
	}

	}

void *reciever(void *ptr)
	{
	char server_response[256];
	char userId[256];
	char password[256];
	int accept_socket=*((int *) ptr);


	recv(accept_socket,&server_response, sizeof(server_response),0);


	

	if(strcmp(server_response,"login")!=0)
	 {
	 recv(accept_socket,&server_response, sizeof(server_response),0);
	 strcpy(userId,server_response);

	 recv(accept_socket,&server_response, sizeof(server_response),0);
	 
	 strcpy(password,server_response);

	 }

	 if(strcmp(server_response,"")!=0)
		 printf("\nData recieved: %s\n", server_response);
	
	
	}*/
