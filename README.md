# FILE_SERVER_CPP

FILE_SERVER_CPP is a FILE_SERVER which was rewrote to C++. The server was written in C++, the client in C

File server is a server where you can save your files and download other files. Connect to server, log in or stay unauthorized.

## How to build

Type in terminal: 

```
./build.sh
```

It will create 2 files. There are `_server` and `_client`

## How to run a server

Type in terminal:

```
./_server <directory>
```

The server create all neccessary files and directory where all files will be saved.

## How to run a client

Type in terminal:

```
./_client <server ip> <port>
```

The port 8808 is a standard port which server uses. If you want to change a port, you can change it in `_server.cpp` file

See `PROTOCOL.txt` to read more.
