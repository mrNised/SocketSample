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
    //We will need a addrinfo to store the result of the DNS
    //request that we make using getaddrinfo
    addrinfo* result = nullptr;

    //A request DNS needs hints to resolve the best address avaiable
    addrinfo hints {};
    memset(&hints, 0, sizeof(hints));
    //We want an IPv4
    hints.ai_family = AF_INET;
    //We want a TCP socket
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    //We make a DNS request asking for the address of localhost on port 45000
    //This request store the information in the result
    int error = getaddrinfo("localhost", "45000", &hints, &result);

    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        return EXIT_FAILURE;
    }

    //Addrinfo (returned by getaddrinfo) is linked list of address.
    //We need to go through it trying to connect to any address returned. Until we find
    //one that is good for us
    SOCKET s = INVALID_SOCKET;
    for(auto* ptr = result; ptr != nullptr ; ptr = ptr->ai_next)
    {
        //We create a matching socket with the one getaddrinfo found
        s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        //Connect asks the OS to connect our socket to a server
        //pass in parameters. This connection will be accepted by
        //the server when this one calls "accept"
        error = connect(s,
                        ptr->ai_addr,
                        ptr->ai_addrlen);

        if(error < 0)
        {
            error = WSAGetLastError();
            std::cout << "There was an error" << std::endl;
            closesocket(s);
            s = INVALID_SOCKET;
            continue;
        }

        break;
    }

    //If the socket is still INVALID this means that the host in unreachable
    if(s == INVALID_SOCKET)
    {
        //No usable address to connect to.
        std::cout << "Host unreachable" << std::endl;
    }

    //Now we are connected to the server
    const char* msg = "Hello World!!!";
    int byteSent = send(s, msg, strlen(msg), 0);

    if(byteSent < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        closesocket(s);
        return EXIT_FAILURE;
    }

    //We finished sending information so we can shutdown
    //our part (the writing or sending part) of the socket
    error = shutdown(s, SD_SEND);

    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        closesocket(s);
        return EXIT_FAILURE;
    }

    std::array<char, 65535> recvBuffer{};
    int byteReceived = 0;

    do {
        byteReceived = recv(s, recvBuffer.data(), recvBuffer.size(), 0);

        if (byteReceived > 0)
        {
            //We received some data
            std::string msgReceived(recvBuffer.data(), byteReceived);
            std::cout << "CLIENT : We received : " << msgReceived << std::endl;
        }
        else if (byteReceived == 0)
        {
            //The socket is closing
            std::cout << "Socket closing..." << std::endl;
        }
        else
        {
            error = WSAGetLastError();
            std::cout << "There was an error" << std::endl;
            closesocket(s);
            return EXIT_FAILURE;
        }
    }while(byteReceived > 0);

    closesocket(s);

    return EXIT_SUCCESS;
}