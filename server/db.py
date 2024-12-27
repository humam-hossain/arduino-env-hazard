# t:125.80
# samples:8
# r_25um:6.68
# ugm3_25um:0.68
# pcs_25um:4176.45
# r_1um:7.16
# ugm3_1um:0.73
# pcs_1um:4473.25

import sqlite3
import random
from datetime import datetime

# Database file path
database_path = "db.sqlite3"

# Function to generate random data closer to the specified values
def generate_random_row():
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    t = round(random.uniform(120.0, 130.0), 2)  # Adjusted temperature range closer to 125.80
    samples = random.randint(5, 15)  # Adjusted sample count closer to 8
    r_25um = round(random.uniform(5.0, 10.0), 2)  # Adjusted R_25um range closer to 6.68
    ugm3_25um = round(random.uniform(0.5, 1.0), 2)  # Adjusted Ugm3_25um range closer to 0.68
    pcs_25um = round(random.uniform(3000.0, 5000.0), 2)  # Adjusted Pcs_25um range closer to 4176.45
    r_1um = round(random.uniform(5.0, 10.0), 2)  # Adjusted R_1um range closer to 7.16
    ugm3_1um = round(random.uniform(0.5, 1.0), 2)  # Adjusted Ugm3_1um range closer to 0.73
    pcs_1um = round(random.uniform(3000.0, 5000.0), 2)  # Adjusted Pcs_1um range closer to 4473.25
    return (timestamp, t, samples, r_25um, ugm3_25um, pcs_25um, r_1um, ugm3_1um, pcs_1um)

# Insert random data into the database
def insert_random_data(num_rows):
    conn = sqlite3.connect(database_path)
    cursor = conn.cursor()

    for _ in range(num_rows):
        random_row = generate_random_row()
        cursor.execute("""
            INSERT INTO api_sensordata (timestamp, t, samples, r_25um, ugm3_25um, pcs_25um, r_1um, ugm3_1um, pcs_1um)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, random_row)

    conn.commit()
    conn.close()
    print(f"{num_rows} rows inserted into the database.")

# Generate and insert 10 new rows
insert_random_data(50)
