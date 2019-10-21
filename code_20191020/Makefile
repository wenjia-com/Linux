.PHONY:all
all:client server
client:udp_client.cc
	g++ -o $@ $^
server:udp_server.cc
	g++ -o $@ $^
.PHONY:clean
clean:
	rm -f client server
