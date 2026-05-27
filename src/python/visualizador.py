import tkinter as tk
import sys, os, random, datetime

sys.path.insert(0, os.path.join(os.path.dirname(__file__)))
import parqueadero as pk

class App:
    def __init__(self, root):
        self.root = root
        self.root.title("Parqueadero")
        self.root.geometry("860x560")
        self.root.resizable(False, False)

        self.park = pk.Parqueadero(20)
        self.park.entrada("ABC123", "08:30:00", 0)
        self.park.entrada("XYZ789", "09:15:00", 3)
        self.park.entrada("DEF456", "10:00:00", 7)

        self.build_ui()
        self.refresh()

    def build_ui(self):
        top = tk.Frame(self.root, bg="#2c3e50", pady=12)
        top.pack(fill=tk.X)

        self.lbl_ocup = tk.Label(top, bg="#2c3e50", fg="#e74c3c", font=("Arial", 12, "bold"))
        self.lbl_libre = tk.Label(top, bg="#2c3e50", fg="#27ae60", font=("Arial", 12, "bold"))
        self.lbl_total = tk.Label(top, text=f"Total: {self.park.getTotal()}",
                                  bg="#2c3e50", fg="white", font=("Arial", 12, "bold"))

        self.lbl_total.pack(side=tk.LEFT, padx=25)
        self.lbl_ocup.pack(side=tk.LEFT, padx=25)
        self.lbl_libre.pack(side=tk.LEFT, padx=25)

        grid = tk.Frame(self.root, bg="white", pady=15)
        grid.pack(fill=tk.BOTH, expand=True, padx=20)

        self.celdas = []
        for i in range(20):
            lbl = tk.Label(grid, width=11, height=4, font=("Arial", 9, "bold"),
                           relief=tk.RAISED, borderwidth=3, fg="white")
            lbl.grid(row=i // 5, column=i % 5, padx=6, pady=6)
            self.celdas.append(lbl)

        bottom = tk.Frame(self.root, bg="white", pady=10)
        bottom.pack(fill=tk.X)

        tk.Button(bottom, text="Entrada", command=self.sim_entrada,
                  bg="#3498db", fg="white", padx=20, pady=8,
                  font=("Arial", 10, "bold")).pack(side=tk.LEFT, padx=20)
        tk.Button(bottom, text="Salida", command=self.sim_salida,
                  bg="#e67e22", fg="white", padx=20, pady=8,
                  font=("Arial", 10, "bold")).pack(side=tk.LEFT)

    def refresh(self):
        self.lbl_ocup.config(text=f"Ocupadas: {self.park.getOcupadas()}")
        self.lbl_libre.config(text=f"Libres: {self.park.getLibres()}")

        for i in range(20):
            est = self.park.getEstado(i)
            placa = self.park.getPlaca(i)
            color = "#27ae60" if est == "LIBRE" else "#e74c3c"
            txt = f"C{i}\n{placa if placa else 'LIBRE'}"
            self.celdas[i].config(bg=color, text=txt)

        self.root.after(1000, self.refresh)

    def sim_entrada(self):
        libres = [i for i in range(20) if self.park.getEstado(i) == "LIBRE"]
        if not libres: return
        celda = random.choice(libres)
        placa = f"SIM{random.randint(100,999)}"
        hora = datetime.datetime.now().strftime("%H:%M:%S")
        self.park.entrada(placa, hora, celda)

    def sim_salida(self):
        ocup = [i for i in range(20) if self.park.getEstado(i) == "OCUPADA"]
        if not ocup: return
        placa = self.park.getPlaca(random.choice(ocup))
        self.park.salida(placa)

if __name__ == "__main__":
    root = tk.Tk()
    App(root)
    root.mainloop()