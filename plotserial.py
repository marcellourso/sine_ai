import serial
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from collections import deque
import re  # Importa la libreria regex per la pulizia della stringa

# Configurazione della porta seriale
ser = serial.Serial(
    port='/dev/ttyACM0',  # Sostituisci con la tua porta seriale
    baudrate=115200,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=1
)

# Buffer circolare per memorizzare i valori acquisiti
data_buffer = deque(maxlen=360)  # Questo buffer memorizza fino a 360 valori del seno
angle_buffer = deque(maxlen=360)  # Questo buffer memorizza fino a 360 angoli

# Configurazione della figura di Matplotlib
fig, ax = plt.subplots()
line, = ax.plot([], [], 'r-')
ax.set_ylim(-1.1, 1.1)
ax.set_xlim(0, 360)
ax.set_xlabel('Angolo (gradi)')
ax.set_ylabel('Valore del Seno')

def clean_data(input_string):
    # Usa una regex per rimuovere eventuali caratteri non numerici (escludendo . e -)
    cleaned_string = re.sub(r'[^0-9.-]', '', input_string)
    # Gestisci eventuali errori di formattazione con numeri duplicati
    return cleaned_string

def update_plot(frame):
    try:
        line_data = ser.readline().decode('utf-8').strip()
        if line_data:
            if "gradi" in line_data and "=" in line_data:
                angle_part, value_part = line_data.split('=')
                angle_str = angle_part.split('(')[-1].split()[0].strip()
                value_str = value_part.strip()

                # Sanifica il valore ricevuto
                value_str = clean_data(value_str)

                # Conversione dei valori
                angle = int(angle_str)
                value = float(value_str)

                print(f"Angolo: {angle}, Valore del Seno: {value}")

                # Aggiungi i nuovi dati al buffer
                if len(angle_buffer) > 0 and angle < angle_buffer[-1]:
                    # Se l'angolo ricomincia da 0, non svuotiamo il buffer, ma continuiamo ad aggiungere i valori
                    # Evita di aggiungere valori con angoli ridotti che potrebbero causare salti nel grafico
                    angle = angle_buffer[-1] + 1
                    if angle >= 360:
                        angle = 0

                angle_buffer.append(angle)
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
