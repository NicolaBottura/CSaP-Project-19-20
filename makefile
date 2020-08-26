FLAGS = -g
EXE = server client
all:$(EXE)
clean:
	rm *.o $(EXE)

server: server.o whiteboard.h whiteboard_sock.o whiteboard_auth.o whiteboard_shm.o whiteboard_sem.o whiteboard_topics.o whiteboard_threads.o whiteboard_messages.o utils.o
	cc $(FLAGS) server.o whiteboard_sock.o whiteboard_auth.o whiteboard_shm.o whiteboard_sem.o whiteboard_topics.o whiteboard_threads.o whiteboard_messages.o utils.o -o server
client: client.o whiteboard.h whiteboard_shm.o whiteboard_sem.o utils.o
	cc $(FLAGS) client.o utils.o whiteboard_shm.o whiteboard_sem.o whiteboard_topics.o whiteboard_threads.o whiteboard_messages.o whiteboard_auth.o -o client

git:
	git add .
	git commit -m "$m"
	git push -u origin master
