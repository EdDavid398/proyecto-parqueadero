#ifndef PARQUEADERO_H
#define PARQUEADERO_H

#include <string>
#include <map>
#include <vector>

struct Vehiculo {
    std::string placa;
    std::string hora;
    int celda;
    Vehiculo() : placa(""), hora(""), celda(-1) {}
    Vehiculo(std::string p, std::string h, int c) : placa(p), hora(h), celda(c) {}
};

class Parqueadero {
private:
    int total;
    std::map<std::string, Vehiculo> autos;
    std::vector<bool> ocupadas;

public:
    Parqueadero(int cap);
    ~Parqueadero();

    bool entrada(std::string placa, std::string hora, int celda);
    bool salida(std::string placa);

    int getLibres() const;
    int getOcupadas() const;
    int getTotal() const;
    std::string getEstado(int celda) const;
    std::string getPlaca(int celda) const;
    std::string getHora(std::string placa) const;
};

#endif