#include <array>
#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>

#ifdef WINDOWS_PLATFORM
//We use a struct to always to WSAStartup and Cleanup only once in the
//lifetime of the program. The static variables below will be created at the program startup
//calling the struct constructor, initializing WinSock. And the variable will be destructed
//at the end of the program calling WASCleanup()
struct WinSockInitializer
{
    WinSockInitializer()
    {
        //On Windows only you need to initialize the Socket Library in version 2.2
        WSAData wsa;
        WSAStartup(MAKEWORD(2,2), &wsa);
    }

    ~WinSockInitializer()
    {
        WSACleanup();
    }
};

static WinSockInitializer __WIN_INIT__ {};
#endif

int main(int argc, char* argv[])
{
    //socket function takes 3 params
    // AF : Address Family IPv4/IPv6 => AF_INET | AF_INET6
    //Type : STREAM => TCP | DGRAM => UDP
    //Protocol : IPPROTO_UDP = UDP | IPPROTO_TCP = TCP  || 0 = auto
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);

    //sockaddr_in represents a IPv4 address.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(45'000);
    serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

    //Once a TCP socket is created,
    // => On a server we must bind to a port and an IP
    int error = bind(s, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        return EXIT_FAILURE;
    }

    //A server must listen for incoming packets
    error = listen(s, 255);
    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        return EXIT_FAILURE;
    }

    //TCP is connection-based. Consequently, we must accept the new connection
    //before being able to talk to a client.
    sockaddr_in clientAddr;
    int clientLength = sizeof clientAddr;
    //memset = Memory Set, we are setting all the memory of clientAddr at 0
    memset(&clientAddr, 0, sizeof clientAddr);

    //Accept accepts a new connection coming from a client. This means that it
    //accepts and acknowledge the "connect" the client made. This function
    //returns a new socket that is used to communicate with this specific
    //client. The parameters are :
    // - The server socket
    // - A pointer to a sockaddr so we can store the client address
    // - A pointer to a size so we can store the client address size
    SOCKET clientSocket = accept(
            s,
            reinterpret_cast<sockaddr*>(&clientAddr),
            &clientLength);

    //Accept on Windows returns an INVALID_SOCKET when there is an error
    if(clientSocket == INVALID_SOCKET)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        closesocket(s);
        return EXIT_FAILURE;
    }

    //We create a buffer to store our received packets
    int byteReceived = 0;
    std::array<char, 65535> recvBuffer{};
    do
    {
        //Contrary to UDP, we use recv instead of recvfrom to receive data. This function
        // takes the CLIENT SOCKET, the buffer, its size and flags (usually 0)
        byteReceived = recv(clientSocket, recvBuffer.data(), recvBuffer.size(), 0);

        if(byteReceived > 0)
        {
            //We received a packet

            //We echo the packet back to the client
            //Like in recv instead of sendto (that we use in UDP) we use send
            //since TCP is connection-based and CLientSocket can only be used
            //to communicate to this specific client.
            error = send(clientSocket, recvBuffer.data(), byteReceived, 0);
            if(error < 0)
            {
                //An error
                error = WSAGetLastError();
                std::cout << "There was an error" << std::endl;
                break;
            }
        }
        else if(byteReceived == 0)
        {
            //The connection is closing
            std::cout << "Connection closing" << std::endl;
        }
        else
        {
            //An error
            error = WSAGetLastError();
            std::cout << "There was an error" << std::endl;
            break;
        }
    }while(byteReceived > 0);

    //We shutdown the send part of our socket. It is considered clean to do
    //before closing the socket. This will allow the client to receive 0 instead of an error
    //when a do subsequent recv.
    error = shutdown(clientSocket, SD_SEND);

    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        closesocket(s);
        return EXIT_FAILURE;
    }

    //We close our client socket
    closesocket(clientSocket);
    //We close our server
    closesocket(s);

    return EXIT_SUCCESS;
}