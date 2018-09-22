/*---------------------------------------------------------------------------------------------------------
 * File name : client.c
 * Author    : Steve Antony Xavier Kennedy
 *
 
 Command line inputs: <IP address> <port number>
 eg:
 ./client 127.0.0.1 9999

 --------------------------------------------------------------------------------------------------------*/
 

// Header section
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<netdb.h>
#include<string.h>
#include<arpa/inet.h>

#define BUFFERSIZE (2048) 


//reating frame structure
typedef struct frame
{
	long int sq_no;
	char data[BUFFERSIZE];
	int length_data;

}Frame;

int main(int argc, char *argv[])
{
        if(argc<3)// passing ip address of server and port number through command line
        {
                fprintf(stderr,"\n Please provide hostname, port number\n");
                exit(1);

        }
        //creating the socket for client 
        int client_socket,portno;
        char buffer[BUFFERSIZE];
	int status=0, ack = 0;; // check the status of sendto and recvfrom

        struct sockaddr_in server_address ;
        client_socket = socket(AF_INET,SOCK_DGRAM,0);

        if(client_socket < 0 ) 
        {
                //printing error message when opening client socket
		printf("\nError opening client socket\n");
                exit(1);
        }
        portno = atoi(argv[2]);// storing the port number from command line argument
        
	//assigning values for the server address structure
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(portno);
	server_address.sin_addr.s_addr = inet_addr(argv[1]);

	int length_address;// for storing the length of server address
	memset(&length_address,0,sizeof(length_address));
 	length_address	= sizeof(server_address);
	
	struct timeval tv;// for socket timeout
	
	while(1)
	{	
                char choice[40];
                char command[20];
                char filename[20];
                long int packets=0;
                int choice_int;
                memset(choice,0,sizeof(choice));
                memset(command,0,sizeof(command));
                memset(filename,0,sizeof(filename));
		 
		Frame frame, recv_frame;
		printf("\n 1. Put \"filename\"\n 2. Get \"filename\"\n 3. Delete \"filename\"\n 4. ls\n 5. exit\n");
		printf("\nEnter the command : "); 
		fgets(choice,40,stdin);
		sscanf(choice,"%s%s",command,filename);// separating command and filename
		
		//converting command to switch integer inputs
		if(strcmp(command,"put")==0)
			choice_int = 1;
		else if(strcmp(command,"get")==0)
			choice_int = 2;
		else if(strcmp(command,"delete")==0)
			choice_int = 3;
		else if(strcmp(command,"ls")==0)
			choice_int = 4;
		else if(strcmp(command,"exit")==0)
			choice_int = 5;
		else
			choice_int =0; // for any invalid command

		long int total_lost = 0;
		sendto(client_socket,&choice_int,sizeof(choice_int),0,(struct sockaddr *)&server_address,length_address);                  
		switch(choice_int)
		{

			case 1: //sending file from client to server - put()
				printf("\nClient: Option selected is put()\n"); 
				
				//creating file pointer
				FILE *fp;	
			    	fp=fopen(filename,"rb");
				int indicate_server =0;// variable for indicating the server about file existance
				long int file_size = 0;
				
				//checking whether file exists in the client database
				if(fp == NULL)
				{
					printf("\nClient: File doesn't exits\n");
					indicate_server = 1;// sets 1 if file doesn't exists
				}
				
				//sending file existance status to server
				
				status = sendto(client_socket,&indicate_server,sizeof(indicate_server),0,(struct sockaddr *)&server_address,length_address);
				

				if(indicate_server==1)
					break;// breaking the loop if file doesn't exists
			
				//setting socket timeout if more than 1 second
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
			
					
				
				// sending the name of the file to server
				ack = 0 , status = 0;
				while(1)
				{
				          sendto(client_socket,filename,sizeof(filename),0,(struct sockaddr *)&server_address,length_address);
                                          status = recvfrom(client_socket,&ack,sizeof(ack),0,(struct sockaddr *)&server_address,(socklen_t *)&length_address);
				          if((status > 0) && ack ==1)
				                  break;
																					                                        printf("\n Resending name of the file\n");
				}

				
				
				// calculating the size of the file to be sent to server
				fseek(fp, 0L, SEEK_END);
				file_size = ftell(fp);
				fseek(fp, 0L, SEEK_SET);
				
				// calculating the number of packets to be sent based on 
				// buffer size and file size
				memset(&packets,0,sizeof(packets));
				
				// if file size is exactly in multiples of BUFFERSIZE
				if((file_size % BUFFERSIZE ) == 0)
				{
					packets = (file_size/BUFFERSIZE);
				}
				// if file size is not exactly a multiple of BUFFERSIZE
				else
				{
					packets = (file_size/BUFFERSIZE)+1 ;
				}
				printf("\nClient: Packets = %ld \n",packets);
				printf("\nClient: File size = %ld ",file_size);	
				
				
				//sending the number of packets
				ack = 0, status = 0;
				while(1)
				{
					sendto(client_socket,&packets,sizeof(packets),0,(struct sockaddr *)&server_address,length_address);
					
					status = recvfrom(client_socket,&ack,sizeof(ack),0,(struct sockaddr *)&server_address,(socklen_t *)\
						&length_address);
					if((status > 0) && ack ==1)
						break;
					printf("\n Resending number of packets\n");
				}


				// sending the file to server
				long int sq_no = 0;	
				for(long int frame_id = 1; frame_id <= packets; frame_id++)

				{
					memset(&(frame),0,sizeof(frame));
					// reading data from file
					frame.length_data = fread(frame.data, 1, BUFFERSIZE , fp);

					int counter =1;

					while(1)
				        {		
						counter++;// counting to break when it is at 100th iteration - timeout
						frame.sq_no = frame_id;
						sendto(client_socket,&frame,sizeof(frame),0,(struct sockaddr *)&server_address,length_address);
						printf("Frame %ld sent", frame_id);
						int recv_result = recvfrom(client_socket, &sq_no, sizeof(sq_no), 0,(struct sockaddr *)&server_address,\
							(socklen_t *)&length_address);
   	                       			//checking if the ack received if of the same sequence number
						if(recv_result > 0 && sq_no == frame_id  )
						{
				        		printf(", Ack %ld received\n", sq_no);
							break;
			              		 }
						else
						{
							printf("\nFrame %ld -> Socket Timeout\n",frame_id);
						}

						printf("\nResending frame %ld--------> \n",frame_id);	
						if(counter == 100)
						{
							// breaking if ack is not received up to 100 retries
							printf("Frame %ld time expired: exceeded 100 iterations\n", frame_id);
							total_lost++;
							break;
						}
					}
				}
			    	printf("\n file sent successfully\n");
				printf("\n Total packets lost %ld\n",total_lost);	
				fclose(fp);
				
				// resetting the socket timeout
				tv.tv_sec = 0;
				tv.tv_usec = 0;
				setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				break;
	
			case 2: //client receiving file from server
				printf("\nClient: Option selected is get()\n"); 
				int file_exists = 0;// variable to indicate whether the file exists in server database

				//setting socket timeout if more than 1 second
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
				

				// sending the name of the file to server
                                ack = 0 , status = 0;
                                while(1)
                                {
	                                sendto(client_socket,filename,sizeof(filename),0,(struct sockaddr *)&server_address,length_address);
	                                                                                                                                                                                                       status = recvfrom(client_socket,&ack,sizeof(ack),0,(struct sockaddr *)&server_address,(socklen_t *)&length_address);
	                                                                                                                                                                                                       if((status > 0) && ack ==1)
		                                break;
		                                                                                                                                                                                              printf("\n Resending name of the file\n");
		                                                                                                                                                                                                                                                                                                                                                             }
		
				
				//resetting socket timeout
				tv.tv_usec = 0;
				tv.tv_sec = 0;
				setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
				
                		
				//checking the file existance and terminating
				recvfrom(client_socket,&file_exists,sizeof(file_exists),0,(struct sockaddr *)&server_address,(socklen_t *)&length_address);

				if(file_exists == 1)
               			{
                    			printf("\nClient: File not found in server\n");
                    			break;
                		}
				
				FILE *fp2;// creating file pointer
        		        fp2=fopen(filename,"wb");
     			
				// setting socket timeout for 1 sec
				tv.tv_usec = 0;
				tv.tv_sec = 1;
				setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				// receving the number of packets from server
				memset(&packets,0,sizeof(packets));
				
				while(1)
				{
					status = recvfrom(client_socket,&packets,sizeof(packets),0,(struct sockaddr *)&server_address, (socklen_t *)\
							&length_address);  
                		
					if((status > 0) && packets >=0)
					{
						ack = 1;
						sendto(client_socket, &ack, sizeof(ack),0,(struct sockaddr *)&server_address, length_address);
						break;
					}
					printf("\n Rereceiving number of packets\n");
				}
				printf("\nPackets = %ld \n",packets);
				
				
				long int counter = 1;// counter for terminating at 100th iteration of resending
				long int received_bytes =0;
				
				//receving data from server
				for(long int frame_id= 1;frame_id <= packets;frame_id++)
				{
					memset(&(recv_frame),0,sizeof(recv_frame));
					
					while(1)
					{
						int recv_result = recvfrom(client_socket, &recv_frame, sizeof(recv_frame), 0,(struct sockaddr*)\
								&server_address,(socklen_t *)&length_address);
						printf("Received frame = %ld , Received length = %d\n", recv_frame.sq_no, recv_frame.length_data);
						
						sendto(client_socket, &recv_frame.sq_no, sizeof(recv_frame.sq_no), 0,(struct sockaddr*)&\
								server_address, length_address);

						if(recv_result > 0 && recv_frame.sq_no==frame_id)
						{
							//writing data to the file
							fwrite(recv_frame.data, 1, recv_frame.length_data, fp2);
							printf("Frame %ld received, Ack sent\n", recv_frame.sq_no);
							received_bytes += recv_frame.length_data;
							break;
		       				}
						printf("\n Resending frame %ld --------->\n",frame_id);
						counter++;
						if(counter == 100)
						{
			                		// breaking the loop if it exceeds 100 retransmits
							printf("Frame %ld time expired: exceeded 100 iterations\n", frame_id);
							total_lost++;
							break;
						}
					}	
				}
							
				printf("\nClient: File received successfully from server\n");
				printf("\nClient: Received bytes = %ld\n",received_bytes);
				printf("\n Total packets lost %ld\n",total_lost);
				printf("******************************\n");	
				fclose(fp2);	
				
				// resetting the socket timeout
				tv.tv_sec = 0;
				tv.tv_usec = 0;
				setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				break;
	
			case 3: //deleting a file from server database
				printf("\nClient: Option selected is to delete a file from server\n");
				int n= sendto(client_socket,filename,sizeof(filename),0,(struct sockaddr *)&server_address,length_address);                                    
				if(n<0)
				{
					printf("Client: error on write\n");	
				}
				
				//checking if the file is successfully deleted
				//if deleted server sets flag as 1
				int flag; 
				recvfrom(client_socket,&flag,sizeof(flag),0,(struct sockaddr *)&server_address, (socklen_t *)&length_address);

				if(flag==0)
				printf("Client: File deleted successfully from server\n");
				else
				printf("Client: File not found in server\n");
				printf("******************************\n");	
				break;

			case 4: //list the files present on server database
				printf("\nClient: Option selected is to list the files on the server database\n"); 
				
				// buffer to hold the list of files
				memset(buffer,0,sizeof(buffer));
				length_address = sizeof(server_address);
				
				//sending filename of the file to be deleted in server
				sendto(client_socket,filename,sizeof(filename),0,(struct sockaddr *)&server_address,length_address); 	
				
				//receiving the list of files from server
				recvfrom(client_socket,buffer,sizeof(buffer),0,(struct sockaddr *)&server_address,(socklen_t *)&length_address);
				printf("List of files present in Server\n");
				
				//printing the list on the screen
				printf("%s",buffer);
				printf("\n******************************\n");
				break;
	
			case 5: // exiting from the application
				printf("Client: You have opted to exit\nThank you\n");
				exit(1);
		
			default://for handling if the user enters any invalid command 
				printf("\nClient: You have entered an invalid choice\nPlease enter the choice again\n");

			}
	}	
//closing client socket	
close(client_socket);
}
