#include <array>
#include <iostream>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <vector>

int main(int argc, char* argv[]) {
#ifdef WINDOWS_PLATFORM
    //On Windows only you need to initialize the Socket Library in version 2.2
    WSAData wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    //socket function takes 3 params
    // AF : Address Family IPv4/IPv6 => AF_INET | AF_INET6
    //Type : STREAM => TCP | DGRAM => UDP
    //Protocol : IPPROTO_UDP = UDP | IPPROTO_TCP = TCP  || 0 = auto
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);

    //sockaddr_in represents a IPv4 address.
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(8888);

    // UINT 16 : 0A0B0C0D
    // Big Endian : 0A 0B 0C 0D
    // Little Endian : 0D 0C 0B 0A
    // All the modern computers are in Little Endian
    // Problem? When network (Internet) was created, the majority of devices
    // were in Big Endian
    // So, all ports and numbers transiting on the network MUST be converted
    // en big endian.
    // htons = Host TO Network Short  => convert a Short (UINT16) from
    // host order (host endianness) to network order (network endianness)

    //Once a UDP socket is created,
    // => On a server we must bind to a port and an IP
    // => On a client we don't give a shit and we send data

    int error = bind(s, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        return EXIT_FAILURE;
    }

    //Since we are a server we just receive data from someone
    //We need a buffer to receive data into
    std::array<char, 65535> buffer;
    //We need a Client Addr to save the address of the client that sent
    // data to us
    sockaddr_in clientAddr;
    int clientLength = sizeof(clientAddr);

    std::vector<sockaddr_in> myClients;

    bool stop = false;
    while(!stop){
        //recvfrom receive data from a client and returns the size of the data received
        int recvLength = recvfrom(s,
                                  buffer.data(),
                                  65535,
                                  0,
                                  reinterpret_cast<sockaddr*>(&clientAddr),
                                  &clientLength);

        if(recvLength < 0)
        {
            std::cout << "We failed to RECV FROM" << std::endl;
            closesocket(s);
            return EXIT_FAILURE;
        }

        //Check if new client
        bool newClient = true;
        for(auto& addr : myClients)
        {
            if(addr.sin_addr.S_un.S_addr == clientAddr.sin_addr.S_un.S_addr){
                newClient = false;
            }
        }

        if(newClient)
        {
            //Add new client
            myClients.push_back(clientAddr);
        }

        //We need to convert Client Addr to string for print
        std::array<char, 256> ip;
        //inet_ntop returns the string representation of clientAddr
        const char* ipClient = inet_ntop(AF_INET, &clientAddr.sin_addr, ip.data(), 256);
        //We print everything
        std::cout << "We received : " <<
                  recvLength << " bytes from : "
                  << ipClient << ":" << ntohs(clientAddr.sin_port) << std::endl;
        std::string msg(buffer.data(), recvLength);
        std::cout << "Message from client : " << msg << std::endl;

        //ntohs : Network TO Host Short => Convert a short from Network Order (Big Endian)
        // to host order (little endian)

        //We send back the message to the client
        for(auto& client : myClients) {
            int clientLen = sizeof(client);
            int byteSent = sendto(s,
                                  buffer.data(),
                                  recvLength,
                                  0,
                                  reinterpret_cast<sockaddr *>(&client),
                                  clientLen);

            if(byteSent < 0)
            {
                std::cout << "We failed to SEND TO" << std::endl;
                closesocket(s);
                return EXIT_FAILURE;
            }
        }
        if(msg == "DISCONNECT"){
            // Mettre le boolean stop à true et close la socket
            stop = true;
            std::cout << "Disconnecting ... " << std::endl;
        }
    }

    //We finished our job so close everything
    closesocket(s);

#ifdef WINDOWS_PLATFORM
    WSACleanup();
#endif
}