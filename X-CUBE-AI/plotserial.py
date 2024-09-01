import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque

# Configurazione della porta seriale
ser = serial.Serial(
    port='/dev/ttyACM0',  # Sostituisci con la tua porta seriale (es. '/dev/ttyUSB0' per Linux)
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

# Buffer circolare per memorizzare i valori acquisiti
data_buffer = deque(maxlen=360)  # Massimo 360 valori, che rappresentano i gradi da 0 a 359

# Configurazione della figura di Matplotlib
fig, ax = plt.subplots()
line, = ax.plot([], [], 'r-')
ax.set_ylim(-1.1, 1.1)
ax.set_xlim(0, 360)
ax.set_xlabel('Angolo (gradi)')
ax.set_ylabel('Valore del Seno')

def update_plot(frame):
    try:
        line_data = ser.readline().decode('utf-8').strip()
        if line_data:
            # Estrai l'angolo e il valore del seno dalla stringa ricevuta
            parts = line_data.split('=')
            if len(parts) == 2:
                angle_str, value_str = parts[0].split()[-2], parts[1].strip()
                angle = int(angle_str.replace('gradi', '').strip())
                value = float(value_str)
                
                data_buffer.append(value)
                
                # Aggiorna i dati del plot
                line.set_data(range(len(data_buffer)), list(data_buffer))
                
    except (ValueError, IndexError) as e:
        print(f"Errore nel parsing dei dati: {e}")

    return line,

# Animazione del plot
ani = animation.FuncAnimation(fig, update_plot, blit=True, interval=50)

plt.show()

# Chiudi la porta seriale quando il plot viene chiuso
ser.close()
