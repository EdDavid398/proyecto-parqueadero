import tkinter as tk
import sys
import os
import threading
import socket
import datetime
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import parqueadero as modulo_parqueadero

SERVIDOR_IP    = "127.0.0.1"
PUERTO_EVENTOS = 8081
BUFFER         = 1024


class Visualizador:
    def __init__(self, ventana):
        self.ventana = ventana
        self.ventana.title("Sistema de Parqueadero")
        self.ventana.geometry("900x600")
        self.ventana.resizable(False, False)
        self.ventana.configure(bg="#1a1a2e")

        self.parqueadero = modulo_parqueadero.Parqueadero(20)
        self.candado     = threading.Lock()
        self.historial   = []

        self.construir_interfaz()
        self.conectar_servidor()
        self.actualizar_vista()

    def construir_interfaz(self):
        barra = tk.Frame(self.ventana, bg="#16213e", pady=10)
        barra.pack(fill=tk.X)

        tk.Label(barra, text="PARQUEADERO", bg="#16213e", fg="#e94560",
                 font=("Consolas", 15, "bold")).pack(side=tk.LEFT, padx=20)

        self.etq_total   = tk.Label(barra, bg="#16213e", fg="#a8dadc",
                                    font=("Consolas", 11, "bold"))
        self.etq_ocupadas = tk.Label(barra, bg="#16213e", fg="#e74c3c",
                                     font=("Consolas", 11, "bold"))
        self.etq_libres  = tk.Label(barra, bg="#16213e", fg="#2ecc71",
                                    font=("Consolas", 11, "bold"))
        self.etq_conexion = tk.Label(barra, bg="#16213e", fg="#f39c12",
                                     font=("Consolas", 10))

        self.etq_total   .pack(side=tk.LEFT, padx=18)
        self.etq_ocupadas.pack(side=tk.LEFT, padx=18)
        self.etq_libres  .pack(side=tk.LEFT, padx=18)
        self.etq_conexion.pack(side=tk.RIGHT, padx=20)

        contenedor_celdas = tk.Frame(self.ventana, bg="#1a1a2e", pady=12)
        contenedor_celdas.pack(fill=tk.BOTH, expand=True, padx=20)

        self.celdas = []
        for i in range(20):
            marco = tk.Frame(contenedor_celdas, bg="#1a1a2e")
            marco.grid(row=i // 5, column=i % 5, padx=5, pady=5)
            etiqueta = tk.Label(marco, width=13, height=5,
                                font=("Consolas", 8, "bold"),
                                relief=tk.FLAT, borderwidth=0, fg="white")
            etiqueta.pack()
            self.celdas.append(etiqueta)

        panel_log = tk.Frame(self.ventana, bg="#0f3460", pady=6)
        panel_log.pack(fill=tk.X, padx=20, pady=(0, 10))

        tk.Label(panel_log, text="REGISTRO DE EVENTOS", bg="#0f3460", fg="#a8dadc",
                 font=("Consolas", 9, "bold")).pack(anchor=tk.W, padx=8)

        self.area_log = tk.Text(panel_log, height=5, bg="#0d0d0d", fg="#00ff41",
                                font=("Consolas", 8), state=tk.DISABLED,
                                relief=tk.FLAT, borderwidth=0)
        self.area_log.pack(fill=tk.X, padx=8, pady=(2, 6))

    def conectar_servidor(self):
        hilo = threading.Thread(target=self.recibir_eventos, daemon=True)
        hilo.start()
        self.registrar_evento("Conectando al servidor...")
        self.etq_conexion.config(text="Conectando...")

    def recibir_eventos(self):
        while True:
            try:
                conexion = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                conexion.connect((SERVIDOR_IP, PUERTO_EVENTOS))
                self.registrar_evento(f"Conectado a {SERVIDOR_IP}:{PUERTO_EVENTOS}")
                self.ventana.after(0, lambda: self.etq_conexion.config(
                    text="Conectado", fg="#2ecc71"))

                while True:
                    datos = conexion.recv(BUFFER)
                    if not datos:
                        break
                    mensaje = datos.decode("utf-8").strip()
                    self.procesar_evento(mensaje)

            except ConnectionRefusedError:
                self.registrar_evento("Servidor no disponible. Reintentando...")
                self.ventana.after(0, lambda: self.etq_conexion.config(
                    text="Sin conexion", fg="#e74c3c"))
            except Exception as error:
                self.registrar_evento(f"Error de conexion: {error}")
            finally:
                try:
                    conexion.close()
                except:
                    pass
                time.sleep(3)

    def procesar_evento(self, mensaje):
        partes = mensaje.split("|")
        if not partes:
            return

        with self.candado:
            tipo = partes[0]

            if tipo == "ENTRADA" and len(partes) >= 4:
                placa = partes[1]
                hora  = partes[2]
                try:
                    celda = int(partes[3])
                except ValueError:
                    return
                ingreso_exitoso = self.parqueadero.entrada(placa, hora, celda)
                estado = "ENTRADA" if ingreso_exitoso else "DUPLICADO"
                self.registrar_evento(f"{estado}  {placa}  Celda {celda:02d}  {hora}")

            elif tipo == "SALIDA" and len(partes) >= 2:
                placa = partes[1]
                salida_exitosa = self.parqueadero.salida(placa)
                estado = "SALIDA" if salida_exitosa else "PLACA NO ENCONTRADA"
                hora_actual = datetime.datetime.now().strftime("%H:%M:%S")
                self.registrar_evento(f"{estado}  {placa}  {hora_actual}")

    def actualizar_vista(self):
        with self.candado:
            num_ocupadas = self.parqueadero.getOcupadas()
            num_libres   = self.parqueadero.getLibres()
            num_total    = self.parqueadero.getTotal()

            self.etq_total   .config(text=f"Total: {num_total}")
            self.etq_ocupadas.config(text=f"Ocupadas: {num_ocupadas}")
            self.etq_libres  .config(text=f"Libres: {num_libres}")

            for i in range(num_total):
                estado = self.parqueadero.getEstado(i)
                placa  = self.parqueadero.getPlaca(i)

                if estado == "LIBRE":
                    color = "#1b4332"
                    texto = f"C{i:02d}\nLIBRE"
                else:
                    hora_ingreso = self.parqueadero.getHora(placa)
                    color = "#7b1d1d"
                    texto = f"C{i:02d}\n{placa}\n{hora_ingreso}"

                self.celdas[i].config(bg=color, text=texto)

        self.ventana.after(500, self.actualizar_vista)

    def registrar_evento(self, texto):
        marca_tiempo = datetime.datetime.now().strftime("%H:%M:%S")
        linea = f"[{marca_tiempo}] {texto}"
        self.historial.append(linea)
        if len(self.historial) > 100:
            self.historial.pop(0)

        def insertar():
            self.area_log.config(state=tk.NORMAL)
            self.area_log.insert(tk.END, linea + "\n")
            self.area_log.see(tk.END)
            self.area_log.config(state=tk.DISABLED)

        self.ventana.after(0, insertar)


if __name__ == "__main__":
    ventana_principal = tk.Tk()
    Visualizador(ventana_principal)
    ventana_principal.mainloop()