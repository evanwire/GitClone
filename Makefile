all: client server

server:
	gcc server.c -lssl -lcrypto -lpthread -o WTFserver

client:
	gcc client.c -lssl -lcrypto -o WTF
test:
	gcc test.c -o WTFtest
clean: client server test
	rm -rf WTFserver
	rm -rf WTF
	rm -rf WTFtest
