# insert 100 accounts into MySQL database
# python insertAccounts.py
# it depends on mysql-connector-python, you should install it first
# pip3 install mysql-connector-python
import mysql.connector # db driver

# --- db config ---
db_config = {
    'host': '127.0.0.1',
    'user': 'admin',
    'password': '111111',
    'database': 'MyGame' 
}

# --- using hash is more safe ---
# import bcrypt
# password_to_store = b'111' # plain password
# hashed_password = bcrypt.hashpw(password_to_store, bcrypt.gensalt())
# # when INSERT  use  hashed_password.decode('utf-8') 

# --- it's for test only, so use plain password ---
plain_password = '111'
role_id_to_use = 0

try:
    conn = mysql.connector.connect(**db_config)
    cursor = conn.cursor()

    sql = "INSERT INTO accounts (account, passwd, roleid) VALUES (%s, %s, %s)"

    accounts_to_insert = []
    for i in range(1, 101):
        account_name = f"ruby{i}"
        # accounts_to_insert.append((account_name, hashed_password.decode('utf-8'), role_id_to_use))
        accounts_to_insert.append((account_name, plain_password, role_id_to_use))

    # use executemany to insert multiple rows
    cursor.executemany(sql, accounts_to_insert)
    conn.commit()

    print(f"Successfully inserted {cursor.rowcount} accounts.")

except mysql.connector.Error as err:
    print(f"Database error: {err}")
    if conn.is_connected():
        conn.rollback() # 
finally:
    if conn.is_connected():
        cursor.close()
        conn.close()
        print("MySQL connection closed.")