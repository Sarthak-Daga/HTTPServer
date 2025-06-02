#include <iostream>
#include <winsock2.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <vector>
#define __port__ 8080
using namespace std;
#pragma comment(lib, "ws2_32.lib")

void addHTML(string &a)
{
  int l = a.length();
  string m = a.substr(l - 4, 4);
  if (m != "html")
  {
    a = a + ".html";
  }
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


vector<char> readBinaryFile(const string &filename)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open())
        return {};

    return vector<char>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}

string getMimeType(const string &path)
{
  size_t dotPos = path.find_last_of('.');
  if (dotPos == string::npos)
    return "application/octet-stream"; // default binary

  string ext = path.substr(dotPos + 1);

  if (ext == "html")
    return "text/html";
  if (ext == "css")
    return "text/css";
  if (ext == "js")
    return "application/javascript";
  if (ext == "png")
    return "image/png";
  if (ext == "jpg" || ext == "jpeg")
    return "image/jpeg";
  if (ext == "ico")
    return "image/x-icon";
  if (ext == "gif")
    return "image/gif";
  if (ext == "svg")
    return "image/svg+xml";
  if (ext == "json")
    return "application/json";

  return "application/octet-stream"; // fallback
}

void handleClient(SOCKET clientSocket)
{

  char buffer[4096];
  int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
  string request(buffer);
  istringstream requestStream(request);
  string method, path, protocol;
  if (!(requestStream >> method >> path >> protocol))
  {
    cout << "Malformed HTTP request" << endl;
    closesocket(clientSocket);
    return;
  }

  if (path == "/")
  {
    path = "/index.html";
  }
  string fileToServe = path.substr(1);
  fileToServe = "www/" + fileToServe;

  vector<char> content = readBinaryFile(fileToServe); // binary read
  string mimeType = getMimeType(fileToServe);

  if (bytesReceived > 0)
  {
    buffer[bytesReceived] = '\0'; // Null-terminate
    cout << "HTTP request received:\n"
         << buffer << endl;

    if (content.empty())
    {
      string notFound =
          "HTTP/1.1 404 Not Found\r\n"
          "Content-Type: text/html\r\n"
          "Content-Length: 22\r\n"
          "Connection: close\r\n"
          "\r\n"
          "<h1>404 Not Found</h1>";
      send(clientSocket, notFound.c_str(), notFound.size(), 0);
    }
    else
    {
      string header =
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: " +
          mimeType + "\r\n"
                     "Content-Length: " +
          to_string(content.size()) + "\r\n"
                                      "Connection: close\r\n"
                                      "\r\n";

      send(clientSocket, header.c_str(), header.size(), 0);
      send(clientSocket, content.data(), content.size(), 0); // Correct binary-safe way
 // binary-safe
    }
  }

  closesocket(clientSocket);
}

int main()
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

  cout << "Server is listening on port " << __port__ << "..." << endl;

  while (true)
  {
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == INVALID_SOCKET)
    {
      cout << "Accept failed." << endl;
      closesocket(serverSocket);
      WSACleanup();
      return 1;
    }

    thread clientThread(handleClient, clientSocket);
    clientThread.detach();
  }

  closesocket(serverSocket);

  WSACleanup();

  return 0;
}