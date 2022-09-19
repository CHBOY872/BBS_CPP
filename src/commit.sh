#!/bin/sh

FILES="*.c *.h *.hpp *.cpp *.sh *.mk makefile PROTOCOL.txt
DbHandler/*.c DbHandler/*.h DbHandler/*.cpp DbHandler/*.hpp
FileDbHandler/*.c FileDbHandler/*.h FileDbHandler/*.hpp FileDbHandler/*.cpp
UserDbHandler/*.c UserDbHandler/*.h UserDbHandler/*.hpp UserDbHandler/*.cpp
Server/*.c Server/*.h Server/*.hpp Server/*.cpp
String/*.c String/*.h String/*.hpp String/*.cpp"

for FILE in $FILES
do 
    git add $FILE
done

if [ -z "$1" ]
then
    git commit
else 
    git commit -m "$1"
fi

if [ "$?" -eq "0" ]
then
    git push
fi