#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include "parqueadero.h"
#include <iostream>
#include <cstring>
#include <sstream>

#define PUERTO 8080
#define BUF 1024

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);

    Parqueadero park(20);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PUERTO);

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    listen(sock, 1);

    std::cout << "Servidor listo en puerto " << PUERTO << "\n";

    SOCKET cli = accept(sock, NULL, NULL);
    std::cout << "Cliente conectado\n";

    char buf[BUF];
    while (true) {
        memset(buf, 0, BUF);
        int n = recv(cli, buf, BUF, 0);
        if (n <= 0) break;

        std::string msg(buf);
        std::stringstream ss(msg);
        std::string cmd, placa, hora;
        int celda;

        std::getline(ss, cmd, '|');
        std::getline(ss, placa, '|');

        if (cmd == "ENTRADA") {
            std::getline(ss, hora, '|');
            ss >> celda;
            bool ok = park.entrada(placa, hora, celda);
            std::cout << (ok ? "[+] " : "[!] ") << placa << " celda " << celda << "\n";
        } else if (cmd == "SALIDA") {
            bool ok = park.salida(placa);
            std::cout << (ok ? "[-] " : "[!] ") << placa << "\n";
        }

        std::string resp = "OK|" + std::to_string(park.getLibres());
        send(cli, resp.c_str(), resp.length(), 0);
    }

    closesocket(cli);
    closesocket(sock);
    WSACleanup();
    return 0;
}