----------------------------------------------------------------------------------------------------------------------------------------------------------
I have created two folders: one for client and one for server.

steps to execute the server file

In the server folder
> make
> ./server <port number>

steps to execute the client file

In the client folder
> make
> ./client <ip address of server> <port number>

- Select a port number greater than 5000.
---------------------------------------------------------------------------------------------------------------------------------------------------------

Implemented the following commands
1) put <filename> - transfers the file <filename> from client to server

on client side
- The client gets the filename from user and checks whether the file is present in client folder. If it is not present, it gives indication to server and displays as "File not found"
- If the file is found, it transfer the file data in the form of frames using stop and wait protocol.

on server side
- If the file is found on the client side, it receives the filename.
- After receiving the file, it calculates the bytes received and displays to the user for comparing.


2) get <filename> - transfers the file <filename> from server to client

on server side
- The server on receiving the filename from client,  checks whether the file is present in server folder. If it is not present, it gives indication to client and displays as "File not found"
- If the file is found, it transfer the file data in the form of frames using stop and wait protocol.

on client side
- The client receives the filename and sends it to the server.
- After receiving the file, it calculates the bytes received and displays to the user for comparing.

3) delete <filename> - deletes the file <filename> from server folder

on client side
- The client gets the filename from user and sends it to the server.

on server side
- The server on receiving the filename, checks if it is present in the server folder. 
- If present, it deletes and sends a message to client.


4) ls - this option lists the files present on the server folder

on client side
- After receiving "ls" command, it requests the server to give the list of files present on the server folder.
- It receives the list of files from the server and displays it to the user.

on server side
- The server writes the list of files present to temporary.log.
- It sends the list to the client for displaying it to the user.

5) exit - exits from the application

It exists from the client and the server program.


---------------------------------------------------------------------------------------------------------------------------------------------------------
Work done on client side

- 	Robust programming technique is implemented on the input command option such that it handles any kind of input including "pressing enter" without crashing. 
-   Socket timeout is set such that it gives timeout if the delay for receiving input from server is more than 1 second.
-   Stop and wait algorithm is implement for sending the data to server such that it sends the packet till it is successfully received by the server and acknowledgement is received by client.
-   The client has been limited to resend the frame for 100 iterations after which it neglects the current frame and sends the next frame.

---------------------------------------------------------------------------------------------------------------------------------------------------------

Work done on server side

Server responds to the requests send by the client.

-   Socket timeout is set such that it gives timeout if the delay for receiving input from server is more than 1 second.
-   Stop and wait algorithm is implement for sending the data to client such that it sends the packet till it is successfully received by the server and acknowledgement is received by client.
-   The client has been limited to resend the frame for 100 iterations after which it neglects the current frame and sends the next frame.

--------------------------------------------------------------------------------------------------------------------------------------------------------

It has been tested with files of 100 MB and it gets transfered succesfully.

--------------------------------------------------------------------------------------------------------------------------------------------------------


