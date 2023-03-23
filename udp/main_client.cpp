#include <array>
#include <iostream>
#include <thread>

#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>

#ifdef __unix__
using SOCKET = int;
#endif

int main() {
#ifdef WINDOWS_PLATFORM
    //On Windows only you need to initialize the Socket Library in version 2.2
    WSAData wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    //socket function takes 3 params
    // AF : Address Family IPv4/IPv6 => AF_INET | AF_INET6
    //Type : STREAM => TCP | DGRAM => UDP
    //Protocol : IPPROTO_UDP = UDP | IPPROTO_TCP = TCP  || 0 = auto
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

/*    //Since we are a client we just send data to someone
    //We need a buffer to receive data into
    const char* data_to_send = "Hello World!!";
    size_t data_length = strlen(data_to_send);*/

    //Server is listening on 127.0.0.1:8888
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    serverAddr.sin_port = htons(8888);

    sockaddr_in iface;
    iface.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &iface.sin_addr);
    iface.sin_port = htons(444444);

    int error = bind(s, reinterpret_cast<sockaddr*>(&iface), sizeof(iface));
    if(error < 0)
    {
        error = WSAGetLastError();
        std::cout << "There was an error" << std::endl;
        return EXIT_FAILURE;
    }

    /*//We send back the message to the client
    int byteSent = sendto(s,
                          data_to_send,
                          data_length,
                          0,
                          reinterpret_cast<sockaddr*>(&serverAddr),
                          sizeof(serverAddr));

    if(byteSent < 0)
    {
        std::cout << "We failed to SEND TO" << std::endl;
        closesocket(s);
        return EXIT_FAILURE;
    }*/

    bool stop = false;
    std::thread receiveThread([&]{
        //Now we receive the serve response
        std::array<char, 65535> buffer;
        sockaddr_in peerAddr;
        int peerAddrLength = sizeof peerAddr;
        //memset = Memory Set, we are setting all the memory of clientAddr at 0
        memset(&peerAddr, 0, sizeof peerAddr);

        while(!stop) {
            //recvfrom receive data from a client and returns the size of the data received
            int recvLength = recvfrom(s,
                                      buffer.data(),
                                      65535,
                                      0,
                                      reinterpret_cast<sockaddr *>(&peerAddr),
                                      &peerAddrLength);

            if (recvLength < 0) {
                int error = WSAGetLastError();
                std::cout << "We failed to RECV FROM" << std::endl;
                closesocket(s);
                return EXIT_FAILURE;
            }

            //We need to convert Client Addr to string for print
            std::array<char, 256> ip;
            //inet_ntop returns the string representation of clientAddr
            const char *ipClient = inet_ntop(AF_INET, &peerAddr.sin_addr, ip.data(), 256);
            //We print everything
            std::cout << std::endl << "We received : " <<
                      recvLength << " bytes from : "
                      << ipClient << ":" << ntohs(peerAddr.sin_port) << std::endl;
            std::string msg(buffer.data(), recvLength);
            std::cout << "Message from server : " << msg << std::endl;
        }
    });

    while(!stop){
        //Lire les messages depuis la console
        std::string messageToSend;
        std::cout << "Enter \"DISCONNECT\" to close the program" << std::endl;
        std::cout << "Enter the message you want to send : ";
        std::cin >> messageToSend;

        //Envoyer le message au serveur

        //Since we are a client we just send data to someone
        //We need a buffer to receive data into
        int data_length = messageToSend.size();


        //We send back the message to the client
        int byteSent = sendto(s,
                              messageToSend.c_str(),
                              data_length,
                              0,
                              reinterpret_cast<sockaddr*>(&serverAddr),
                              sizeof(serverAddr));

        if(byteSent < 0)
        {
            std::cout << "We failed to SEND TO" << std::endl;
            closesocket(s);
            return EXIT_FAILURE;
        }

        //Si le client tape DISCONNECT quitter le while, mettre le boolean stop à true et close la socket
        if(messageToSend == "DISCONNECT"){
            // Mettre le boolean stop à true et close la socket
            stop = true;
            std::cout << "Disconnecting ... " << std::endl;

            //ATTENTION : en C++ un thread doit être join avant d'être détruit
            //regarde les fonctions is_joinable() et join() avant de quitter la main
            if(receiveThread.joinable()){
                receiveThread.join();
            }
        }
    }

    //We finished our job so close everything
    closesocket(s);

#ifdef WINDOWS_PLATFORM
    WSACleanup();
#endif
}
