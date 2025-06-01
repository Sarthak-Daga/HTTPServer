@echo off

g++ server.cpp -o server -lws2_32

echo "Server Online!!"

server.exe

pause