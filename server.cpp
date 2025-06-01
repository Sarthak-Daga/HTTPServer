#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <sstream>
#define __port__ 8080
using namespace std;
#pragma comment(lib, "ws2_32.lib")

void addHTML(string& a){
  int l = a.length();
  string m = a.substr(l-4,4);
  if(m!="html"){a = a + ".html";}
}

string readFile(const string &filename)
{
  ifstream file(filename);
  if (!file.is_open())
    return "";

  stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

int main()
{

  while (true)
  {
    WSAData wsadata;
    int result = WSAStartup(MAKEWORD(2, 2), &wsadata);
    if (result != 0)
    {
      cout << "WSAStartup failed: " << result << endl;
      return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
      cout << "Socket creation failed." << endl;
      WSACleanup(); // Cleanup before exiting
      return 1;
    }

    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(__port__);

    if (bind(serverSocket, (sockaddr *)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
    {
      cout << "Bind failed." << endl;
      closesocket(serverSocket); // Close socket before exiting
      WSACleanup();
      return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
      cout << "Listen failed." << endl;
      closesocket(serverSocket);
      WSACleanup();
      return 1;
    }

    cout << "Server is listening on port "<<__port__<<"..." << endl;

    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET)
    {
      cout << "Accept failed." << endl;
      closesocket(serverSocket);
      WSACleanup();
      return 1;
    }

    char buffer[4096];

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    string request(buffer);
    istringstream requestStream(request);
    string method , path , protocol;
    requestStream>>method>>path>>protocol;

    if(path=="/") {path = "/index.html";}
    string fileToServe = path.substr(1);
    fileToServe = "www/"+fileToServe;
    addHTML(fileToServe);
    string html = readFile(fileToServe);
    if (bytesReceived > 0)
    {
      buffer[bytesReceived] = '\0'; // Null-terminate the buffer to treat it as a string
      cout << "HTTP request received:\n"
           << buffer << endl;

      if (html.empty())
      {
        html = "<h1>404 Not Found</h1>";
      }

      string httpResponse =
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Connection: close\r\n"
          "\r\n" +
          html;

      send(clientSocket, httpResponse.c_str(), httpResponse.size(), 0);
    }

    closesocket(clientSocket);

    closesocket(serverSocket);

    WSACleanup();
  }

  return 0;
}