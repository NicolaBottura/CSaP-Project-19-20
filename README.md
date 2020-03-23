# CSaP---Project-19-20
#
#
# @Author Nicola Bottura 

Write a "whiteboard" application, where a community of users could interact and exchange messages.

A server will wait for connections, on a INET socket(1), forking new processes to handle interactions with clients(one process per client).

A client can issues five operations to the sever:

	1) Authenticate;
	2) List, subscribe and create "topics";
	3) Append a message to one "topic";
	4) See the status of sent message (received/published);
	5) Receive (and also reply to) messages, by posting to topic.

Note: use processes (not threads) and SYSV IPCs (not POSIX).
      The data structure should reside in shared memory and protected
      with SYSV semaphores.

Once a message is publishe to a "topic", all subscribed users will be able to get a copy of it (*IDEA: scarica una copia in locale*).
Messages cannot be edited or deleted once sent/published.
Users are added/deleted to the system by an external administrator which manages credentials.


*(1): Internet sockets

							23/03/2020
