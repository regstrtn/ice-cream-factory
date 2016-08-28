all: master
master: 
	gcc -w -o master process.c -lpthread
