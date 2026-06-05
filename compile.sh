#!/bin/bash
set -e

echo "=== Compilacion proyecto parqueadero ==="

PYTHON_INC=$(python3 -c "import sysconfig; print(sysconfig.get_path('include'))")
PYTHON_VER=$(python3 -c "import sys; v=sys.version_info; print(f'{v.major}{v.minor}')")
PYTHON_VER_DOT=$(python3 -c "import sys; v=sys.version_info; print(f'{v.major}.{v.minor}')")
PYTHON_PREFIX=$(python3 -c "import sys; print(sys.prefix)")

echo "Python include : $PYTHON_INC"
echo "Python version : $PYTHON_VER_DOT"

echo ""
echo "[1/4] Generando wrapper SWIG..."
swig -c++ -python -outdir src/python src/cpp/parqueadero.i

echo "[2/4] Compilando libreria dinamica..."
g++ -fPIC -c src/cpp/parqueadero.cpp \
    -I"$PYTHON_INC" -Isrc/cpp \
    -std=c++11 -o build/parqueadero.o

g++ -fPIC -c src/cpp/parqueadero_wrap.cxx \
    -I"$PYTHON_INC" -Isrc/cpp \
    -std=c++11 -o build/parqueadero_wrap.o

PYTHON_DLL=""
for ruta in \
    "$PYTHON_PREFIX/python3${PYTHON_VER}.dll" \
    "$PYTHON_PREFIX/python${PYTHON_VER}.dll" \
    "/c/Windows/System32/python${PYTHON_VER}.dll" \
    "/mingw64/lib/libpython${PYTHON_VER_DOT}.dll.a" \
    "/usr/lib/libpython${PYTHON_VER_DOT}.dll.a"; do
    if [ -f "$ruta" ]; then
        PYTHON_DLL="$ruta"
        break
    fi
done

if [ -n "$PYTHON_DLL" ] && [[ "$PYTHON_DLL" == *.dll.a ]]; then
    g++ -shared build/parqueadero.o build/parqueadero_wrap.o \
        "$PYTHON_DLL" \
        -o src/python/_parqueadero.pyd
else
    g++ -shared build/parqueadero.o build/parqueadero_wrap.o \
        -L"$PYTHON_PREFIX" -lpython${PYTHON_VER} \
        -o src/python/_parqueadero.pyd
fi
echo "    _parqueadero.pyd generado"

echo "[3/4] Compilando servidor..."
g++ -std=c++11 \
    src/cpp/servidor.cpp \
    src/cpp/parqueadero.cpp \
    -Isrc/cpp \
    -o build/servidor.exe \
    -lws2_32 -lwsock32
echo "    servidor.exe generado"

echo "[4/4] Compilando cliente..."
g++ -std=c++11 \
    src/cpp/cliente.cpp \
    -Isrc/cpp \
    -o build/cliente.exe \
    -lws2_32 -lwsock32
echo "    cliente.exe generado"

echo ""
echo "=== Compilacion exitosa ==="
echo ""
echo "Orden de ejecucion:"
echo "  1. build/servidor.exe"
echo "  2. python3 src/python/visualizador.py"
echo "  3. build/cliente.exe"
echo ""
ls -lh build/ src/python/_parqueadero.pyd 2>/dev/null || true