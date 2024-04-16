import socket

load_balancer_address = 'localhost'
load_balancer_port = 8080


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((load_balancer_address, load_balancer_port))
s.sendall(b'game')
data = s.recv(1024)
print(data.decode())

while True:
    line = input()
    s.sendall(line.encode())
    if line == 'quit':
        break
    data = s.recv(1024).decode()
    print(data)
    if 'won' in data or 'lost' in data:
        break


s.close()
