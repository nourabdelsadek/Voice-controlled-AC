import threading
import speech_recognition as sr
import pyaudio
import audioop
import ttkbootstrap as ttk
from ttkbootstrap.constants import *
from word2number import w2n
import re
import tkinter as tk
import serial
import math

COM_PORT = 6
BAUD_RATE = 9600

CHUNK = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100

def get_temp_color(temp):
    # Clamp temp between 0 and 50 for color calculation
    t = max(0, min(50, temp)) / 50.0
    
    # Cold (Blue) -> Comfortable (Greenish) -> Hot (Red)
    if t < 0.5:
        # Interpolate Blue to Green
        ratio = t * 2
        r = int(52 + (46 - 52) * ratio)
        g = int(152 + (204 - 152) * ratio)
        b = int(219 + (113 - 219) * ratio)
    else:
        # Interpolate Green to Red
        ratio = (t - 0.5) * 2
        r = int(46 + (231 - 46) * ratio)
        g = int(204 + (76 - 204) * ratio)
        b = int(113 + (60 - 113) * ratio)
        
    return f'#{r:02x}{g:02x}{b:02x}'

class WeatherCanvas(tk.Canvas):
    def __init__(self, master, width=100, height=100, **kwargs):
        super().__init__(master, width=width, height=height, highlightthickness=0, **kwargs)
        self.width = width
        self.height = height
        self.cx = width // 2
        self.cy = height // 2

    def draw_weather(self, temp):
        self.delete("all")
        if temp >= 30: 
            self._draw_sun()
        elif temp <= 15:
            self._draw_snow()
        else:
            self._draw_cloud()

    def _draw_sun(self):
        self.create_oval(self.cx-20, self.cy-20, self.cx+20, self.cy+20, fill="#f1c40f", outline="#f39c12", width=2)
        for i in range(0, 360, 45):
            rad = math.radians(i)
            x1 = self.cx + math.cos(rad) * 25
            y1 = self.cy + math.sin(rad) * 25
            x2 = self.cx + math.cos(rad) * 35
            y2 = self.cy + math.sin(rad) * 35
            self.create_line(x1, y1, x2, y2, fill="#f39c12", width=3)

    def _draw_snow(self):
        self.create_text(self.cx, self.cy, text="❄", font=("Arial", 60), fill="white")

    def _draw_cloud(self):
        self.create_oval(self.cx-30, self.cy-10, self.cx+10, self.cy+20, fill="#ecf0f1", outline="")
        self.create_oval(self.cx-10, self.cy-20, self.cx+30, self.cy+15, fill="#ecf0f1", outline="")

class FanWidget(tk.Canvas):
    def __init__(self, master, width=180, height=180, **kwargs):
        super().__init__(master, width=width, height=height, highlightthickness=0, **kwargs)
        self.cx = width // 2
        self.cy = height // 2
        self.angle = 0
        self.direction = 'S'
        self.speed = 0
        self.blade_color = "#95a5a6"
        self.draw_static_frame()
        self.animate()

    def draw_static_frame(self):
        self.create_oval(self.cx-80, self.cy-80, self.cx+80, self.cy+80, outline="#555", width=8)
        self.create_oval(self.cx-5, self.cy-5, self.cx+5, self.cy+5, fill="#333")

    def set_state(self, direction, speed):
        self.direction = direction
        self.speed = speed

    def animate(self):
        self.delete("blade")
        
        step = 0
        if self.direction == 'F': 
            step = (self.speed / 255.0) * 30 + 2
            self.blade_color = "#2ecc71" 
        elif self.direction == 'B':
            step = -((self.speed / 255.0) * 30 + 2)
            self.blade_color = "#e74c3c" 
        else:
            self.blade_color = "#95a5a6"

        self.angle = (self.angle + step) % 360

        for i in range(3):
            blade_angle = math.radians(self.angle + (i * 120))
            x_tip = self.cx + math.cos(blade_angle) * 70
            y_tip = self.cy + math.sin(blade_angle) * 70
            
            x_l = self.cx + math.cos(blade_angle - 0.3) * 20
            y_l = self.cy + math.sin(blade_angle - 0.3) * 20
            
            x_r = self.cx + math.cos(blade_angle + 0.3) * 20
            y_r = self.cy + math.sin(blade_angle + 0.3) * 20
            
            self.create_polygon(self.cx, self.cy, x_l, y_l, x_tip, y_tip, x_r, y_r, 
                              fill=self.blade_color, tags="blade", outline="black")
        
        self.after(50, self.animate)

class SmartHVACApp(ttk.Window):
    def __init__(self):
        super().__init__(themename="superhero")
        self.title("Smart HVAC Controller")
        self.geometry("1000x600")
        
        self.recognizer = sr.Recognizer()
        self.is_recording = False
        self.frames = []
        self.ser = None
        
        self.target_temp = 25
        self.inside_temp = 25
        self.outside_temp = 35 
        
        self.setup_ui()
        
        self.serial_thread = threading.Thread(target=self.listen_to_arduino, daemon=True)
        self.serial_thread.start()

    def setup_ui(self):
        self.container = tk.Frame(self)
        self.container.pack(fill=BOTH, expand=YES)
        
        self.left_pane = tk.Frame(self.container, bg="#3498db", width=400)
        self.left_pane.pack(side=LEFT, fill=BOTH, expand=YES)
        self.left_pane.pack_propagate(False)

        self.center_pane = tk.Frame(self.container, bg="#2c3e50", width=200)
        self.center_pane.pack(side=LEFT, fill=Y)
        self.center_pane.pack_propagate(False)

        self.right_pane = tk.Frame(self.container, bg="#e74c3c", width=400)
        self.right_pane.pack(side=LEFT, fill=BOTH, expand=YES)
        self.right_pane.pack_propagate(False)

        # --- LEFT PANE (OUTSIDE) ---
        tk.Label(self.left_pane, text="OUTSIDE", font=("Helvetica", 18, "bold"), bg=self.left_pane["bg"], fg="white").pack(pady=20)
        self.weather_icon_out = WeatherCanvas(self.left_pane, width=100, height=100, bg=self.left_pane["bg"])
        self.weather_icon_out.pack(pady=10)
        self.lbl_out_temp = tk.Label(self.left_pane, text="--°C", font=("Helvetica", 60, "bold"), bg=self.left_pane["bg"], fg="white")
        self.lbl_out_temp.pack(pady=20)
        self.lbl_out_status = tk.Label(self.left_pane, text="Status", font=("Helvetica", 14), bg=self.left_pane["bg"], fg="#ecf0f1")
        self.lbl_out_status.pack()

        # --- CENTER PANE (FAN & CONTROLS) ---
        tk.Label(self.center_pane, text="AIRFLOW", font=("Helvetica", 12, "bold"), bg="#2c3e50", fg="#bdc3c7").pack(pady=20)
        self.fan = FanWidget(self.center_pane, bg="#2c3e50")
        self.fan.pack(pady=10)
        
        self.lbl_fan_status = tk.Label(self.center_pane, text="IDLE", font=("Courier", 10, "bold"), bg="#2c3e50", fg="#f39c12")
        self.lbl_fan_status.pack(pady=5)

        self.mic_bar = ttk.Progressbar(self.center_pane, length=150, maximum=20000, bootstyle="info")
        self.mic_bar.pack(pady=30)
        
        self.btn_record = ttk.Button(self.center_pane, text="WAITING", bootstyle="secondary", state="disabled", width=15)
        self.btn_record.pack(pady=10)

        # --- RIGHT PANE (INSIDE) ---
        tk.Label(self.right_pane, text="INSIDE", font=("Helvetica", 18, "bold"), bg=self.right_pane["bg"], fg="white").pack(pady=20)
        self.lbl_target = tk.Label(self.right_pane, text="Target: 25°C", font=("Helvetica", 16), bg=self.right_pane["bg"], fg="#ecf0f1")
        self.lbl_target.pack(pady=5)
        
        self.lbl_in_temp = tk.Label(self.right_pane, text="--°C", font=("Helvetica", 60, "bold"), bg=self.right_pane["bg"], fg="white")
        self.lbl_in_temp.pack(pady=20)
        
        self.thermometer_bar = ttk.Progressbar(self.right_pane, orient=VERTICAL, length=200, maximum=50, bootstyle="light")
        self.thermometer_bar.pack(pady=10)

    def listen_to_arduino(self):
        try:
            self.ser = serial.Serial(f'COM{COM_PORT}', BAUD_RATE, timeout=1)
            print(f"Connected to COM{COM_PORT}")
            while True:
                if self.ser.in_waiting > 0:
                    line = self.ser.readline().decode('utf-8').strip()
                    if line == "Start":
                        self.after(0, self.start_recording)
                    elif line == "Stop":
                        self.after(0, self.stop_recording)
                    else:
                        try:
                            # Format: state, target, inside, outside, speed
                            parts = line.split(',')
                            if len(parts) == 5:
                                self.after(0, lambda p=parts: self.update_data(p))
                        except ValueError:
                            pass
        except Exception as e:
            print(f"Serial Error: {e}")

    def update_data(self, parts):
        m_state = parts[0].strip()
        t_target = float(parts[1].strip())
        t_in = float(parts[2].strip())
        t_out = float(parts[3].strip())
        speed = int(parts[4].strip())

        self.target_temp = int(t_target)
        self.inside_temp = int(t_in)
        self.outside_temp = int(t_out)

        # Update Left (Outside)
        color_out = get_temp_color(self.outside_temp)
        self.left_pane.configure(bg=color_out)
        self.lbl_out_temp.configure(text=f"{self.outside_temp}°C", bg=color_out)
        self.lbl_out_status.configure(bg=color_out, text="Hot Environment" if self.outside_temp > 25 else "Cold Environment")
        self.weather_icon_out.configure(bg=color_out)
        self.weather_icon_out.draw_weather(self.outside_temp)
        
        # Update Right (Inside)
        color_in = get_temp_color(self.inside_temp)
        self.right_pane.configure(bg=color_in)
        self.lbl_in_temp.configure(text=f"{self.inside_temp}°C", bg=color_in)
        self.lbl_target.configure(text=f"Target: {self.target_temp}°C", bg=color_in)
        self.thermometer_bar.configure(value=self.inside_temp)

        # Update Fan
        self.fan.set_state(m_state, speed)
        if m_state == 'F':
            self.lbl_fan_status.configure(text=f"Fan\nIN <- OUT\nSpeed: {speed}")
        elif m_state == 'B':
            self.lbl_fan_status.configure(text=f"Hood\nIN -> OUT\nSpeed: {speed}")
        else:
            self.lbl_fan_status.configure(text="Stopped")

    def start_recording(self):
        if not self.is_recording:
            self.is_recording = True
            self.frames = []
            self.btn_record.configure(bootstyle="danger", text="LISTENING...", state="normal")
            threading.Thread(target=self.record_thread, daemon=True).start()

    def stop_recording(self):
        self.after(200, self._stop_flag)

    def _stop_flag(self):
        self.is_recording = False
        self.btn_record.configure(bootstyle="secondary", text="PROCESSING...", state="disabled")

    def record_thread(self):
        p = pyaudio.PyAudio()
        try:
            stream = p.open(format=FORMAT, channels=CHANNELS, rate=RATE, input=True, frames_per_buffer=CHUNK)
        except OSError:
            return

        while self.is_recording:
            try:
                data = stream.read(CHUNK, exception_on_overflow=False)
                self.frames.append(data)
                rms = audioop.rms(data, 2)
                self.after(0, lambda v=rms: self.mic_bar.configure(value=v))
            except Exception:
                break
        
        stream.stop_stream()
        stream.close()
        p.terminate()
        self.mic_bar.configure(value=0)
        self.process_audio()

    def process_audio(self):
        raw_data = b''.join(self.frames)
        audio_source = sr.AudioData(raw_data, RATE, 2)
        try:
            text = self.recognizer.recognize_google(audio_source).lower()
            print(f"Recognized: {text}")
            self.process_command_locally(text)
        except Exception:
            pass
        self.after(0, lambda: self.btn_record.configure(text="WAITING"))

    def process_command_locally(self, text):
        new_temp = None
        text = text.replace("to", "two").replace("too", "two").replace("for", "four")
        try:
            number_found = w2n.word_to_num(text)
            if number_found:
                new_temp = int(number_found)
        except ValueError:
            digits = re.findall(r'\d+', text)
            if digits:
                new_temp = int("".join(digits))

        if new_temp is not None:
            self.target_temp = new_temp
            self.after(0, lambda: self.lbl_target.configure(text=f"Target: {self.target_temp}°C"))
            
            if self.ser and self.ser.is_open:
                try:
                    to_send = f"{new_temp}\n"
                    if len(to_send)  == 2:
                        to_send = '0' + to_send
                    self.ser.write(to_send.encode())
                except Exception:
                    pass

if __name__ == "__main__":
    app = SmartHVACApp()
    app.mainloop()