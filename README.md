# Sistema de Parqueadero

Simulador de gestión de parqueadero con sockets TCP y librería dinámica SWIG.

## Requisitos
- g++, python3, swig, make

## Compilar
```bash
bash compile.sh
```

## Ejecutar

Terminal 1:
```bash
./build/servidor.exe
```
Terminal 2:
```bash
./build/cliente.exe
```
Terminal 3:
```bash
python3 src/python/visualizador.py
```

## Protocolo
- `ENTRADA|placa|hora|celda`
- `SALIDA|placa`

## Estructura
- C++: lógica + sockets TCP
- SWIG: expone C++ a Python
- Python: interfaz gráfica Tkinter