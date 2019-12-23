import sqlite3
from sys import argv
conn = sqlite3.connect("BMSTU.db") # или :memory: чтобы сохранить в RAM
cursor = conn.cursor()

id=argv[1]
name=argv[2]
information=argv[3]
n_photo=argv[4]

info=[(id,name,information,n_photo)]

cursor.executemany("INSERT INTO BMSTU VALUES (?,?,?,?)", info)
conn.commit()
