import serial
import time
import csv
import os
from datetime import datetime

# Replace 'COMx' (Windows) or '/dev/ttyUSB0' (Linux/Mac) with your Arduino's serial port
arduino_port = '/dev/ttyUSB0'  # Example for Linux
baud_rate = 115200  # Match the Arduino's baud rate

# Create the data folder if it doesn't exist
data_folder = 'data'
os.makedirs(data_folder, exist_ok=True)

# Generate a timestamped filename
timestamp = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
csv_file = os.path.join(data_folder, f'arduino_data_{timestamp}.csv')

try:
    # Open serial connection
    ser = serial.Serial(arduino_port, baud_rate, timeout=1)
    print(f"Connected to {arduino_port}")
    
    time.sleep(2)  # Allow time for the connection to establish
    
    # Open CSV file for writing
    with open(csv_file, mode='w', newline='') as file:
        writer = None
        current_fieldnames = None  # Track current field names
        while True:
            # Read data from the Arduino
            if ser.in_waiting > 0:
                # Decode data, replacing invalid characters
                line = ser.readline().decode('utf-8', errors='replace').strip()
                print(line)
                
                # Parse the line into a dictionary
                try:
                    data_dict = dict(item.split(':') for item in line.split())
                except ValueError:
                    # Skip invalid lines
                    continue
                
                # Skip empty dictionaries
                if not data_dict:
                    continue
                
                # Check if the CSV writer needs to be updated with new headers
                if current_fieldnames is None or set(data_dict.keys()) != set(current_fieldnames):
                    current_fieldnames = data_dict.keys()
                    file.seek(0, 0)  # Reset file to overwrite with new headers
                    writer = csv.DictWriter(file, fieldnames=current_fieldnames)
                    writer.writeheader()  # Write new header
                
                # Write the data to the CSV
                writer.writerow(data_dict)
        
except serial.SerialException as e:
    print(f"[ERROR] {e}")
except KeyboardInterrupt:
    print("\n[INFO] Exiting...")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("[INFO] Serial port closed.")
        print(f"[INFO] Data saved to {csv_file}")
