#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include <iostream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <iomanip>

#define PUERTO 8080
#define BUF 1024

std::string genPlaca() {
    std::string s = "";
    for (int i = 0; i < 3; i++) s += char('A' + rand() % 26);
    for (int i = 0; i < 3; i++) s += char('0' + rand() % 10);
    return s;
}

std::string getHora() {
    time_t t = time(0);
    struct tm* lt = localtime(&t);
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << lt->tm_hour << ":"
       << std::setfill('0') << std::setw(2) << lt->tm_min << ":"
       << std::setfill('0') << std::setw(2) << lt->tm_sec;
    return ss.str();
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2), &wsa);
    srand(time(0));

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in srv = {};
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PUERTO);
    srv.sin_addr.s_addr = inet_addr("192.168.20.31");

    if (connect(sock, (struct sockaddr*)&srv, sizeof(srv)) == SOCKET_ERROR) {
        std::cerr << "No conectó al servidor\n";
        WSACleanup();
        return 1;
    }
    std::cout << "Conectado\n";

    std::vector<std::string> activos;
    char buf[BUF];

    for (int i = 0; i < 20; i++) {
        std::string msg;

        if (activos.empty() || rand() % 100 < 75) {
            std::string placa = genPlaca();
            int celda = rand() % 20;
            msg = "ENTRADA|" + placa + "|" + getHora() + "|" + std::to_string(celda);
            activos.push_back(placa);
            std::cout << "-> " << placa << "\n";
        } else {
            int idx = rand() % activos.size();
            msg = "SALIDA|" + activos[idx];
            std::cout << "<- " << activos[idx] << "\n";
            activos.erase(activos.begin() + idx);
        }

        send(sock, msg.c_str(), msg.length(), 0);
        memset(buf, 0, BUF);
        recv(sock, buf, BUF, 0);
        Sleep(2000 + rand() % 3000);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}