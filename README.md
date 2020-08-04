# CSaP-Project-19/20
# Nicola Bottura 

A "whiteboard" application, where a community of users could interact and exchange messages.
A server will wait for connections, on a INET socket(Internet Socket), forking new processes to handle interactions with clients(one process per client).

A client can issues five operations to the sever:

	1) Authenticate;
	2) List, subscribe and create "topics" - like a discussion on a forum;
	3) Append a message to one "topic" - comment under the topic;
	4) See the status of sent message (received/published);
	5) Receive (and also reply to) messages, by posting to topic.

Note: 	use processes (not threads) and SYSV IPCs (not POSIX).
      	The data structure should reside in shared memory and protected
      	with SYSV semaphores.
	Once a message is publishe to a "topic", all subscribed users will be able to get a copy of it.
	Messages cannot be edited or deleted once sent/published.
	Users are added/deleted to the system by an external administrator which manages credentials.

[CLIENT COMMANDS]
	1. authenticate -> *reads and sends user name and password to server*
	2. list \[Messages|Topics] ->	*Gets list of messages, read or unread, ordered by topics*
					*Lists available|subscribed topics*
	3. get \[message#] -> *receives and display message on user console*
	4. status \[message#] -> *displays the status of a specific message*
	5. reply \[message#] -> *appends a new message to a thread (in a topic)*
	6. create \[topic] -> *creates a new topic (the user will be the owner)*
	7. appen \[topic] \[thread] -> *appends a (new) message to a new thread in a topic*
	8. subscribe \[topic] -> *insert the user in the list of recipients for this topic*
	9. delete \[topic] -> *[only if owner of the topic], deletes the topic and all messages*
