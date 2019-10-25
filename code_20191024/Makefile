

.PHONY:all
all:tcpserver tcpclient

tcpclient:tcpclient.cc
	g++ -o $@ $^
tcpserver:tcpserver.cc
	g++ -o $@ $^
.PHONY:clean
clean:
	rm -f tcpclient tcpserver

