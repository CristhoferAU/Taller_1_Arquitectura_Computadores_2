import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import os

RESULTS_DIR = "results"

class BenchmarkViewer(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Benchmark Viewer")
        self.geometry("1600x500")

        self.benchmarks = [d for d in os.listdir(RESULTS_DIR) if os.path.isdir(os.path.join(RESULTS_DIR, d))]

        self.combo = ttk.Combobox(self, values=self.benchmarks, state="readonly", width=40)
        self.combo.pack(pady=10)
        self.combo.bind("<<ComboboxSelected>>", self.load_images)

        self.img_frame = tk.Frame(self)
        self.img_frame.pack()

        self.frames = [tk.Label(self.img_frame) for _ in range(3)]
        for frame in self.frames:
            frame.pack(side="left", padx=10)

    def load_images(self, event):
        bench = self.combo.get()
        bench_path = os.path.join(RESULTS_DIR, bench)

        images = ["tiempo.png", "speedup.png", "eficiencia.png"]
        self.photo_images = []

        for frame, img_name in zip(self.frames, images):
            img_path = os.path.join(bench_path, img_name)
            if os.path.exists(img_path):
                pil_img = Image.open(img_path)
                
                w, h = pil_img.size
                new_w = 525
                new_h = int(h * (new_w / w))
                pil_img = pil_img.resize((new_w, new_h))

                photo = ImageTk.PhotoImage(pil_img)
                frame.config(image=photo)
                frame.image = photo
            else:
                frame.config(text=f"No se encontró {img_name}", image='')

if __name__ == "__main__":
    app = BenchmarkViewer()
    app.mainloop()
