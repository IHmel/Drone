import sqlite3
from sys import argv

conn =sqlite3.connect('BMSTU.db')

cursor = conn.cursor()

if argv[1] == 'help':
        print('insert ', 'INSERT INTO BMSTU VALUES (ID,NAME,INFO)','\n'\
          'edit ', 'UPDATE BMSTU SET what = new  WHERE what = old','\n'\
          'delete ', 'DELETE FROM BMSTU WHERE what = old')

if argv[1] == 'insert':
    args=[(argv[2], argv[3], argv[4])]
    print("INSERT INTO BMSTU VALUES (?,?,?)",args)
    cursor.executemany("INSERT INTO BMSTU VALUES (?,?,?)",args)
    conn.commit()

if argv[1] == 'edit':
    sql = """UPDATE BMSTU SET """ + argv[2] + """ = """+argv[3]+ """ WHERE """+argv[4] +"""= """+ argv[5]
    print(sql)
    cursor.execute(sql)
    conn.commit()

if argv[1] == 'delete':
    sql="""DELETE FROM BMSTU WHERE """+argv[2]+""" = """+argv[3]
    print(sql)
    cursor.execute(sql)
    conn.commit()
