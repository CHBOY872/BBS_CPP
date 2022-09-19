#!/bin/sh

cd src;
./build.sh
cd ..
if [ "$?" -eq "0" ]
then 
    mv src/_server .
    mv src/_client .
    chmod +x _server
    chmod +x _client
fi