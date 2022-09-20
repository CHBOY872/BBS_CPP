CC=gcc
CFLAGS=-Wall

CPP=g++
CPPFLAGS=-Wall

DEBUG=-g
RELEASE=-s

ANSI=-ansi
ISOSTD=

SRCMODULES=String/String.cpp Server/Server.cpp \
UserDbHandler/UserDbHandler.cpp FileDbHandler/FileDbHandler.cpp 
HDRMODULES=$(SRCMODULES:.cpp=.hpp)
OBJMODULES=$(SRCMODULES:.cpp=.o)

%.o: %.cpp %.hpp
	$(CPP) $(CPPFLAGS) $(DEBUG) $(ANSI) $< -c -o $@

_server: _server.cpp $(OBJMODULES)
	$(CPP) $(CPPFLAGS) $(DEBUG) $(ANSI) $^ -o $@

_client: _client.c
	$(CC) $(CFLAGS) $(DEBUG) $(ANSI) $< -o $@

ifneq (clean, $(MAKECMDGOALS))
-include deps.mk
endif

deps.mk: $(SRCMODULES)
	$(CC) -MM $^ > $@

all:
	make _server
	make _client