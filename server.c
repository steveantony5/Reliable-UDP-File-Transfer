/*-------------------------------------------------------------------------------
 *File name : server.c
 *Author    : Steve Antony Xavier Kennedy

 *Command line inputs : port number
 eg:
 ./server 9898

-------------------------------------------------------------------------------*/

// Header section
#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<unistd.h>
#include<ctype.h>
#include<string.h>

#define BUFFERSIZE (2048) 

//creating the frame structure
typedef struct frame
{
    long int sq_no;
    char data[BUFFERSIZE];
    int length_data;

}Frame;


int main(int argc, char *argv[])
{
	if(argc<2) // getting the port number through command line argument
	{
		printf("\n Please provide port number\n");
		exit(1);

	}
	//creating the socket
	int server_socket,portno;
	char buffer[BUFFERSIZE]; // for holding values
	int ack =0, status = 0; // check the status of sendto and recvfrom
	struct sockaddr_in server_address, to_address;
	server_socket = socket(AF_INET,SOCK_DGRAM,0);
	if(server_socket<0)
	{
		printf("\nServer: Error opening server socket\n");
		exit(1);
	}
	bzero((char *)&server_address,sizeof(server_address));
	portno = atoi(argv[1]);// storing the port number in a variable

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr	= INADDR_ANY;
	server_address.sin_port = htons(portno);

	int length_address;// for storing the length of address
	Frame recv_frame, frame;

	struct timeval tv;// for setting socket timeout

	//bind the server socket with the remote client socket
	if(bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address))<0)
	{
		printf("\nBinding failed\n");
	}

	printf("\nServer: Waiting for command from client\n");
	
	char filename_server[20];
	while(1)
	{
		long int packets=0;

		//getting the command choice from client
		int choice;
		memset(&choice,0,sizeof(choice));
		recvfrom(server_socket,&choice,sizeof(choice),0,(struct sockaddr *)&to_address,(socklen_t *)&length_address);
		
		switch(choice)
		{
			case 1: //sending file from client to server
				printf("\n Client option is put()\n");
				int file_exists = 0;//variable for checking the existance of file
				
				//setting socket timeout to 0
				tv.tv_sec=0;
                                tv.tv_usec=0;
                                setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				// checking whether the file exists in client side
				recvfrom(server_socket,&file_exists,sizeof(file_exists),0,(struct sockaddr *)&to_address,(socklen_t *)&length_address);
				if(file_exists == 1)
				{
					// Terminating if the file doesn't exists on the client database
					printf("\nServer: Requested file not present in server\n");
					break;
				}
			
				// setting socket timeout for
                                tv.tv_sec = 1;
                                tv.tv_usec = 0;
                                setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

	
				//getting the filename from client
				memset(filename_server,0,sizeof(filename_server));
				status = 0, ack = 0;
				while(1)
                                {
                                        status = recvfrom(server_socket,filename_server,sizeof(filename_server),0,(struct sockaddr *)\
						&to_address,(socklen_t *)&length_address);

                                        if((status > 0) && packets >= 0)
                                        {
                                                ack =1;
                                                sendto(server_socket, &ack, sizeof(ack), 0,(struct sockaddr*)&to_address, length_address);
                                                break;
                                        }
                                }
				

				FILE *fp;// creating the file pointer
				fp=fopen(filename_server,"wb");


				// getting the number of packets from client
				memset(&packets,0,sizeof(packets));
				ack =0, status =0;	
				while(1)
				{
					status = recvfrom(server_socket,&packets,sizeof(packets),0,(struct sockaddr *)&to_address,(socklen_t *)&\
						length_address);
				
					if((status > 0) && packets >= 0)
					{
						ack =1;
						sendto(server_socket, &ack, sizeof(ack), 0,(struct sockaddr*)&to_address, length_address);
						break;
					}
				}
	
				printf("\nServer: Packets =  %ld ",packets);
				

				long int received_bytes = 0;
				int counter = 1;// counter for terminting at 100th iteration

				//receiving file data from client
				for(long int frame_id = 1; frame_id <= packets; frame_id++)
				{
					memset((&recv_frame),0,sizeof(recv_frame));

					while(1)
					{
						int recv_result = recvfrom(server_socket, &recv_frame, sizeof(recv_frame), 0,(struct sockaddr*)
							&to_address,(socklen_t *)&length_address);
						printf("\nReceived frame = %ld, Received length = %d\n", recv_frame.sq_no, recv_frame.length_data);

						//sending acknowledgement sequence number
						sendto(server_socket, &recv_frame.sq_no, sizeof(recv_frame.sq_no), 0,(struct sockaddr*)\
							&to_address, length_address);
						
						if(recv_result > 0 && recv_frame.sq_no==frame_id )
						{

							//writing the received data to file
							fwrite(recv_frame.data, 1, recv_frame.length_data, fp);
							printf("Frame %ld received, Ack sent\n", recv_frame.sq_no);
							received_bytes += recv_frame.length_data;// calculating the recived file size
							break;
		            			}
						
						printf("\nResending frame %ld -------> \n",frame_id);
						counter++;
						if(counter == 100)
						{
			                		//breaking the loop if it fails even at 100th resend
							printf("Frame %ld time expired: exceeded 100 counts\n", frame_id);
							break;
						}
					}	
				}
				printf("\nServer: File received\n");

				printf("Server: Received bytes = %ld\n",received_bytes);
				printf("\nServer: Completed Client request\n");
				fclose(fp);
				
				//resetting the socket timeout
				tv.tv_sec=0;
                                tv.tv_usec=0;
                                setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				break;

			case 2: // sending file from server to client
				printf("Server: Client option is get()\n");
				long int file_size = 0;
				
				FILE *fp2;// creating file pointer

				memset(filename_server,0,sizeof(filename_server));
				
				//setting socket timeout
				tv.tv_sec=1;
				tv.tv_usec=0;	
				setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				
				//getting the filename from client
                                memset(filename_server,0,sizeof(filename_server));
                                status = 0, ack = 0;
                                while(1)
                                {
                                        status = recvfrom(server_socket,filename_server,sizeof(filename_server),0,(struct sockaddr *)\
						&to_address,(socklen_t *)&length_address);

                                        if((status > 0) && packets >= 0)
                                        {
                                                ack =1;
                                                sendto(server_socket, &ack, sizeof(ack), 0,(struct sockaddr*)&to_address, length_address);
                                                break;
                                        }
                                }

	
			
				fp2=fopen(filename_server,"rb");
				int indicate_client = 0;// to indicate client if the file is not found

				// checking if the file exits
				if(fp2 == NULL)
				{
					printf("\nServer: File doesn't exits\n");
					indicate_client = 1;
				}

				// resetting the socket timeout
                                tv.tv_sec = 0;
                                tv.tv_usec = 0;
                                setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));

				sendto(server_socket,&indicate_client,sizeof(indicate_client),0,(struct sockaddr *)&to_address,length_address);
				if(indicate_client==1)
				break;//terminate if the file doesn't exists

				// calculating th file size
				fseek(fp2, 0L, SEEK_END);
				file_size = ftell(fp2);
				fseek(fp2, 0L, SEEK_SET);
				

				// calculating the number of packets
				memset(&packets,0,sizeof(packets));
				
				//when the file size is a multiple of BUFFERSIZE
				if((file_size % BUFFERSIZE )==0)
				{
					packets = (file_size/BUFFERSIZE);
				}
				
				//when the file name is not a multiple of BUFFERSIZE
				else
				{
					packets = (file_size/BUFFERSIZE)+1 ;
				}
				
				// setting the socket timeout as 1 sec
                                tv.tv_sec = 1;
                                tv.tv_usec = 0;
                                setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));


				while(1)
				{
					sendto(server_socket,&packets,sizeof(packets),0,(struct sockaddr *)&to_address,length_address);
					
					status = recvfrom(server_socket,&ack,sizeof(ack),0,(struct sockaddr *)&to_address,(socklen_t *)&length_address);
					if((status > 0) && ack == 1)	
						break;
					printf("\n Resending number of packets\n");
				}	
			


				printf("\nServer: Packets = %ld \n",packets);
				printf("\nServer: File size = %ld ",file_size);

				
				//sending the file to client
				long int sq_no = 0;
				for(long int frame_id  = 1; frame_id <= packets;frame_id++)
				{
					memset(&(frame),0,sizeof(frame));
					frame.length_data = fread(frame.data, 1, BUFFERSIZE, fp2);
					
					long int counter = 1;//counter for terminating if it exceeds 100 counts
					
					while(1)
			       	 	{		
						counter++;
						frame.sq_no = frame_id;
						sendto(server_socket,&frame,sizeof(frame),0,(struct sockaddr *)&to_address,length_address);
						printf("Frame %ld sent\n", frame_id);
						int recv_sqno = recvfrom(server_socket, &sq_no, sizeof(sq_no), 0,(struct sockaddr *)\
							&to_address,(socklen_t *)&length_address);
                       				//receiving acknowledgement
						if(recv_sqno > 0 && sq_no == frame_id )
						{
			        			printf("Ack %ld received\n", sq_no);
							break;
		               		 	}
						else
						{
				        		printf("Frame %ld -> Socket Timeout\n", frame_id);
						}
						printf("\nResending frame %ld -------->\n",frame_id);
						if(counter == 100)
						{
							// terminating when it fails even at 100 tries
							printf("Frame %ld time expired: exceeded 100 iterations\n", frame_id);
							break;
						}
					}
				}
	
				printf("\n Server : File sent to client successfully\n");
				fclose(fp2);

				// resetting the socket
				tv.tv_sec = 0;
				tv.tv_usec = 0;
				setsockopt(server_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(struct timeval));
				printf("\nServer: Completed Client request\n");

				break;

			case 3: //deleting a file on server database
				printf("\nServer: Client option is to delete a file from server\n");

				int flag, recv=0;


				memset(filename_server,0, sizeof(filename_server));

				if((recv =recvfrom(server_socket,filename_server,sizeof(filename_server),0,(struct sockaddr *)\
					&to_address,(socklen_t *)&length_address)) == -1)
			       	{
					printf(" Fail on receive ");
				}		

				// flag to indicate if it deleted successfully
				flag=remove(filename_server);
				sendto(server_socket,&flag,sizeof(flag),0,(struct sockaddr *)&to_address,length_address);
				printf("\nServer: Completed client request\n");
				break;

			case 4: //listing the files in the server database
				printf("\nServer: Client option is to search the list of files in server");
				FILE *fp_ls;// creating a file pointer

				//sending the list command to system
				system("ls >> temporary.log");

				fp_ls=fopen("temporary.log","rb");
				
				memset(buffer,0,sizeof(buffer));
				
				//reading the file list from the log file
				fread(buffer,1,BUFFERSIZE,fp_ls);

				fclose(fp_ls);
				recvfrom(server_socket,filename_server,sizeof(filename_server),0,(struct sockaddr *)&to_address,(socklen_t *)&length_address);
				sendto(server_socket,buffer,sizeof(buffer),0,(struct sockaddr *)&to_address,length_address);
				
				//removing the temporary log file creating
				system("rm temporary.log");
				printf("\nServer: Completed client request\n");
				break;

			case 5: // exiting from the application
				printf("\nExited from server\nThank you\n");
				exit(1);
		}
	}
	// closing server socket
	close(server_socket);
}
