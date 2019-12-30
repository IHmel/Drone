import sqlite3
 
conn = sqlite3.connect("BMSTU.db") # или :memory: чтобы сохранить в RAM
cursor = conn.cursor()
 
# Создание таблицы
cursor.execute("""CREATE TABLE BMSTU
                  (ID text, NAME text, INFO text)
               """)
