//
// Created by qinrui on 2019/12/2.
//

#ifndef TEST02_SERVICE_HPP
#define TEST02_SERVICE_HPP

#include <iostream>
#include <thread>
#include <Winsock2.h>
#include <inaddr.h>

using namespace std;

struct Callback {

    virtual void handle(const SOCKET &socket, const std::vector<char> &v_data) = 0;
};


struct WinService {

    SOCKET sockSrv;
    Callback *lpCallback;

    WinService(int port, Callback &callback) {
        lpCallback = &callback;

        WSADATA wsa;
        if (WSAStartup(MAKEWORD(1, 1), &wsa) != 0) {
            std::cerr << "[error] : WSAStartup() fail" << endl;
        }
        sockSrv = socket(AF_INET, SOCK_STREAM, 0);
        SOCKADDR_IN addrSrv;
        addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
        addrSrv.sin_family = AF_INET;
        addrSrv.sin_port = htons(port);
        ::bind(sockSrv, (SOCKADDR *) &addrSrv, sizeof(SOCKADDR_IN));
    }

    void handle(SOCKET socket) {
        while (true) {
            char c_magic[4];
            int recv_len = recv(socket, c_magic, 4, MSG_WAITALL);
            if (recv_len == 0 || recv_len == SOCKET_ERROR) {
                std::cerr << "[warning] : client socket has closed" << endl;
                closesocket(socket);
                break;
            }

            unsigned int magic = ntohl(*(unsigned int *) &c_magic[0]);
            if (magic == 0x12345678) {
                char c_len[4];
                recv(socket, c_len, 4, 0);
                unsigned int len = ntohl(*(unsigned int *) &c_len[0]);
                char *c_data = new char[len];
                recv_len = recv(socket, c_data, len, MSG_WAITALL);

                std::vector<char> v_data;
                v_data.assign(c_data, c_data + len);
                delete[] c_data;

                lpCallback->handle(socket, v_data);

            } else {
                cout << "[error] : magic error" << endl;
                closesocket(socket);
                break;
            }

        }
    }

    void run() {
        if (listen(sockSrv, 5) != 0) {
            cout << "[error] : listen() fail" << endl;
            return;
        }

        while (true) {
            SOCKADDR_IN addrClient;
            int len = sizeof(SOCKADDR);
            SOCKET socket = accept(sockSrv, (SOCKADDR *) &addrClient, &len);

            std::thread th(&WinService::handle, this, socket);
            th.join();
        }
    }

    void strat() {
        std::thread th(&WinService::run, this);
        th.join();
    }

    ~WinService() {
        WSACleanup();
    }

};

#endif //TEST02_SERVICE_HPP
