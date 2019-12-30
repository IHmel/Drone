
import sqlite3
import sys

conn =sqlite3.connect('BMSTU.db')

cursor = conn.cursor()

photo_name = sys.argv[1]


sql = "SELECT * FROM BMSTU 
cursor.execute(sql, [(photo_name)])
while True:
      row =cursor.fetchone()

      if row == None:
           break

      print( row[0], row[1], row[2])
