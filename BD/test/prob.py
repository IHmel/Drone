# Подключаем библиотеки
import sqlite3 as lite
import sys
# Функция открытия изображения в бинарном режиме
def readImage(filename):
    try:
        fin = open(filename, "rb")
        img = fin.read()
        return img        
    except IOError as e:
        # В случае ошибки, выводим ее текст
        print ("Error %d %s" % (e.args[0],e.args[1]))
        sys.exit(1)
 
    finally:
        if fin:
            # Закрываем подключение с файлом
            fin.close()
 
try:
    # Открываем базу данных
    con = lite.connect('test.db')
    cur = con.cursor()
    # Получаем бинарные данные нашего файла
    data = readImage("сборка1.jpg")
    # Конвертируем данные
    binary = lite.Binary(data)
    # Готовим запрос в базу
    cur.execute("INSERT INTO Images(Data) VALUES (?)", (binary,) )
    # Выполняем запрос
    con.commit()    
    
# В случаи ошибки выводим ее текст.
except lite.Error as e:
    if con:
        con.rollback()
        
    print ("Error %s"  %( e.args[0]))
    sys.exit(1)
    
finally:
    if con:
    # Закрываем подключение с базой данных
        con.close()
