#define _WIN32_WINNT 0x0601
#include <winsock2.h>
#include "parqueadero.h"
#include <iostream>
#include <cstring>
#include <sstream>
#include <thread>
#include <mutex>

#define PUERTO_CLIENTES    8080
#define PUERTO_VISUALIZADOR 8081
#define BUFFER 1024

Parqueadero parqueadero(20);
std::mutex  mutex_parqueadero;

SOCKET socket_visualizador = INVALID_SOCKET;
std::mutex mutex_visualizador;

void reenviarAlVisualizador(const std::string& mensaje) {
    std::lock_guard<std::mutex> bloqueo(mutex_visualizador);
    if (socket_visualizador == INVALID_SOCKET) return;
    send(socket_visualizador, mensaje.c_str(), (int)mensaje.length(), 0);
}

void escucharVisualizador() {
    SOCKET servidor_viz = socket(AF_INET, SOCK_STREAM, 0);
    int opcion = 1;
    setsockopt(servidor_viz, SOL_SOCKET, SO_REUSEADDR, (char*)&opcion, sizeof(opcion));

    struct sockaddr_in direccion = {};
    direccion.sin_family      = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port        = htons(PUERTO_VISUALIZADOR);

    bind(servidor_viz, (struct sockaddr*)&direccion, sizeof(direccion));
    listen(servidor_viz, 1);
    std::cout << "Esperando visualizador en puerto " << PUERTO_VISUALIZADOR << "\n";

    while (true) {
        SOCKET cliente_viz = accept(servidor_viz, NULL, NULL);
        {
            std::lock_guard<std::mutex> bloqueo(mutex_visualizador);
            if (socket_visualizador != INVALID_SOCKET)
                closesocket(socket_visualizador);
            socket_visualizador = cliente_viz;
        }
        std::cout << "Visualizador conectado\n";
    }
}

void atenderCliente(SOCKET cliente) {
    char buffer[BUFFER];

    while (true) {
        memset(buffer, 0, BUFFER);
        int bytes = recv(cliente, buffer, BUFFER, 0);
        if (bytes <= 0) break;

        std::string mensaje(buffer);
        std::stringstream flujo(mensaje);
        std::string tipo, placa, hora;
        int celda = -1;

        std::getline(flujo, tipo,  '|');
        std::getline(flujo, placa, '|');

        {
            std::lock_guard<std::mutex> bloqueo(mutex_parqueadero);

            if (tipo == "ENTRADA") {
                std::getline(flujo, hora, '|');
                flujo >> celda;
                bool registrado = parqueadero.entrada(placa, hora, celda);
                std::cout << (registrado ? "[ENTRADA] " : "[DUPLICADO] ")
                          << placa << " celda " << celda << "\n";

            } else if (tipo == "SALIDA") {
                bool liberado = parqueadero.salida(placa);
                std::cout << (liberado ? "[SALIDA] " : "[NO ENCONTRADO] ")
                          << placa << "\n";
            }
        }

        reenviarAlVisualizador(mensaje);

        std::string respuesta = "OK|" + std::to_string(parqueadero.getLibres());
        send(cliente, respuesta.c_str(), (int)respuesta.length(), 0);
    }

    closesocket(cliente);
    std::cout << "Cliente desconectado\n";
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    std::thread(escucharVisualizador).detach();

    SOCKET servidor = socket(AF_INET, SOCK_STREAM, 0);
    int opcion = 1;
    setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, (char*)&opcion, sizeof(opcion));

    struct sockaddr_in direccion = {};
    direccion.sin_family      = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port        = htons(PUERTO_CLIENTES);

    bind(servidor, (struct sockaddr*)&direccion, sizeof(direccion));
    listen(servidor, 5);
    std::cout << "Servidor activo en puerto " << PUERTO_CLIENTES << "\n";

    while (true) {
        SOCKET cliente = accept(servidor, NULL, NULL);
        std::cout << "Cliente conectado\n";
        std::thread(atenderCliente, cliente).detach();
    }

    closesocket(servidor);
    WSACleanup();
    return 0;
}