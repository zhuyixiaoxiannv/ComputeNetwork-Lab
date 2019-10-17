# ComputeNetwork-Lab
This is the code for class ComputeNetwork's lab
##What this project do
This project uses the socket in C++(most in C code) under Windows, to realize and simulate the connect between the server and client  

The function is very easy, I agree.

But it includes all the work that need to do when you want to connect with another computer(a server or another client).

##Start
###Using
Under windows——if you need Linux ,please rewrite and I'm not sure you can use it.

Open the Client\Debug\Client.exe and Server\Server\Debug\Server.exe  
——of course ,you can open several client to connect with the server

You can enter number 1-7 under client and enter 'q' or 'Q' under Server

Client:

	1、start connect
	you need to know the server's IP and the port
	which is set 441
	of course ,you can just use one computer
	and the IP is 127.0.0.1

	2、close connect
	3、ask the server for the current time
	4、ask the server for the server's name
	5、ask for the list of the client linked to the server
	6、send a message to another client
	which should get the client list first
	7、close this client

Server

	You can see some information
	and enter q or Q will quit the Server and close the connect
###Edit
Using VS2017 under windows

Of course,you can use the cpp file directly

You need also try to download and bulid an environment of pthread