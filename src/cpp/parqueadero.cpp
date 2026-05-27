#include "parqueadero.h"
#include <iostream>

Parqueadero::Parqueadero(int cap) : total(cap) {
    ocupadas.resize(cap, false);
}

Parqueadero::~Parqueadero() { autos.clear(); }

bool Parqueadero::entrada(std::string placa, std::string hora, int celda) {
    if (celda < 0 || celda >= total) return false;
    if (ocupadas[celda]) return false;
    if (autos.find(placa) != autos.end()) return false;
    autos[placa] = Vehiculo(placa, hora, celda);
    ocupadas[celda] = true;
    return true;
}

bool Parqueadero::salida(std::string placa) {
    auto it = autos.find(placa);
    if (it == autos.end()) return false;
    ocupadas[it->second.celda] = false;
    autos.erase(it);
    return true;
}

int Parqueadero::getLibres() const {
    int c = 0;
    for (bool o : ocupadas) if (!o) c++;
    return c;
}

int Parqueadero::getOcupadas() const { return autos.size(); }
int Parqueadero::getTotal() const { return total; }

std::string Parqueadero::getEstado(int celda) const {
    if (celda < 0 || celda >= total) return "INVALIDA";
    return ocupadas[celda] ? "OCUPADA" : "LIBRE";
}

std::string Parqueadero::getPlaca(int celda) const {
    for (auto& p : autos)
        if (p.second.celda == celda) return p.first;
    return "";
}

std::string Parqueadero::getHora(std::string placa) const {
    auto it = autos.find(placa);
    return it != autos.end() ? it->second.hora : "";
}