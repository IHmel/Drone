import sqlite3
from sys import argv
 
conn = sqlite3.connect("BMSTU.db")
cursor = conn.cursor()

sql =argv[1]
 
cursor.execute(sql)
conn.commit()
