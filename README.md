# CSaP-Project-19/20 - Nicola Bottura 

A "whiteboard" application, where a community of users could interact and exchange messages.
A server will wait for connections, on a INET socket, forking new processes to handle interactions with clients(one process per client).

When a new client connects to the server, the server will ask for a pair of credentials to log in and after the check, it will be able to execute some operations.
These operation are:
  1. Create a new topic - the creator of the topic will be automatically subscribed to the topic
  2. List all the topics specifying if the current client is subscribed or not
  3. Delete a topic - only if the client executing this operation is also the owner - and all the related threads and messages
  4. Reply to a thread - add a new message under a thread in a topic
  5. Create a new thread under a topic
  6. Display the content of a topic, so for the chosen topic, all the threads with the related messages
  7. Subscribe to a topic
  8. Show unread messages - if there are new messages that are not read for a subscribed topic, list them
  9. Unsubscribe a topic - remove a topic from the subscription of a user
 
The credentials, the topic, threads and messages are stored in structs in SYSV shared memory with also a counter used to manage the total number of elements for each structure.
Two semaphores will control the operations for the clients, one for the authentication and one for all the creation/removal operations related to topics, threads and messages.

# How to use
To compile the program just launch the compiler script:
``` 
./compile.sh 
```
which will execute a ```make clean``` and then a  ```make -f ./makefile```

Once compiled, to start the server, execute the server executable like this:
```
./server <listening-port>
```
while for the client
```
./client <server-ip> <server-port>
```

# Known problems
Because of the send/recv and recv/send combination for the communication between client and server, I noticed that, when sending too much data(~16 topics for example) with just a send() function, the output will be truncated on the client console and to complete the list of elements it will need to press ENTER again, so the server will receive too much data in response than the expected amount and the client will terminate.
A possible solution is the commented line 143 in whiteboard_topics.c, which will send all the topics with a ping() function instead of just a send().
But in this way, the client will press ENTER to display each topic and if they are many it can be tedious.
Since this was just a project for an exam, this code may be not that much robust, for example there are no checks for the total size of shared memory used(it's easy to solve but I'm lazy, sorry).
