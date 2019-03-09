all: producer validate

producer: producer.c 
	$(CC) -g -O0 producer.c -o producer -I/usr/local/include -lfcontainer
	
validate: validate.c 
	$(CC) -g -O0 validate.c -o validate -lfcontainer
	
clean:
	rm -f producer validate 
