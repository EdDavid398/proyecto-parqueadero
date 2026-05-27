#!/bin/bash

echo "=== Rutas de Python ==="
PYTHON_INC=$(python3 -c "import sysconfig; print(sysconfig.get_path('include'))")
PYTHON_LIB=$(python3 -c "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))")
PYTHON_VER=$(python3 -c "import sys; v=sys.version_info; print(f'python{v.major}{v.minor}')")

echo "Include: $PYTHON_INC"
echo "Lib: $PYTHON_LIB"
echo "Version: $PYTHON_VER"

echo ""
echo "=== Compilando SWIG ==="
swig -c++ -python src/cpp/parqueadero.i
if [ $? -ne 0 ]; then echo "ERROR en SWIG"; exit 1; fi

g++ -fPIC -c src/cpp/parqueadero.cpp -I"$PYTHON_INC" -std=c++11
g++ -fPIC -c src/cpp/parqueadero_wrap.cxx -I"$PYTHON_INC" -std=c++11
g++ -shared parqueadero.o parqueadero_wrap.o \
    -L"$PYTHON_LIB" -l"$PYTHON_VER" \
    -o src/python/_parqueadero.pyd
mv parqueadero.py src/python/ 2>/dev/null || true
echo "SWIG OK"

echo ""
echo "=== Compilando servidor ==="
g++ -std=c++11 \
    src/cpp/servidor.cpp src/cpp/parqueadero.cpp \
    -o build/servidor.exe \
    -lws2_32 -lwsock32
if [ $? -ne 0 ]; then echo "ERROR servidor"; exit 1; fi
echo "Servidor OK"

echo ""
echo "=== Compilando cliente ==="
g++ -std=c++11 \
    src/cpp/cliente.cpp \
    -o build/cliente.exe \
    -lws2_32 -lwsock32
if [ $? -ne 0 ]; then echo "ERROR cliente"; exit 1; fi
echo "Cliente OK"

echo ""
echo "=== Todo listo ==="
ls build/