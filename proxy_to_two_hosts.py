# Proxy incoming request to two servers

import os,sys,thread,socket

BACKLOG = 50            # how many pending connections queue will hold
MAX_DATA_RECV = 999999  # max number of bytes we receive at once

SERVER1 = "localhost"
PORT1 = 8081
SERVER2 = "localhost"
PORT2 = 8082

def main():

    # check the length of command running
    if (len(sys.argv) < 2):
        print "No port given, using port 8080."
        port = 8080
    else:
        port = int(sys.argv[1])

    host = ''               # blank for localhost
    print "Proxy Server Running on ",host,":",port

    try:
        # create a socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        # associate the socket to host and port
        s.bind((host, port))

        # listenning
        s.listen(BACKLOG)

    except socket.error, (value, message):
        if s:
            s.close()
        print "Could not open socket:", message
        sys.exit(1)

    # get the connection from client
    while 1:
        conn, client_addr = s.accept()

        # create a thread to handle request
        thread.start_new_thread(proxy_thread, (conn, client_addr))

    s.close()

def proxy_thread(conn, client_addr):

    # get the request from browser
    request = conn.recv(MAX_DATA_RECV)

    # Server 1
    try:
        # create a socket to connect to the web server
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER1, PORT1))
        s.send(request)         # send request to webserver

        conn.send("200");
        s.close()
        conn.close()
    except socket.error, (value, message):
        if s:
            s.close()
        if conn:
            conn.close()

    # Server 2
    try:
        # create a socket to connect to the web server
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((SERVER2, PORT2))
        s.send(request)         # send request to webserver

        conn.send("200");
        s.close()
        conn.close()
    except socket.error, (value, message):
        if s:
            s.close()
        if conn:
            conn.close()

if __name__ == '__main__':
    main()

