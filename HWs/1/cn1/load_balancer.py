
#sara azarnoush 98170668

import socket
import threading

game_servers = []
game_servers_cv = threading.Condition()
users = []


def handle_request(c, a):
    try:
        data = c.recv(1024)
        request = data.decode()
        if request.startswith('server'):
            game_server_port = int(request.split()[1])
            game_servers_cv.acquire()
            game_servers.append((a[0], game_server_port))
            game_servers_cv.notify_all()
            game_servers_cv.release()
            print('game server added')
        elif request.startswith('game'):
            users.append(a)
            while True:
                game_servers_cv.acquire()
                while len(game_servers) == 0:
                    game_servers_cv.wait()
                game_server = game_servers.pop()
                game_servers_cv.release()
                print('game server found')
                print(game_server)
                game_server_s = socket.socket(
                    socket.AF_INET, socket.SOCK_STREAM)
                try:
                    game_server_s.connect((game_server[0], game_server[1]))
                    break
                except:
                    pass
            try:
                while True:
                    data = game_server_s.recv(1024)
                    if data is None or len(data) == 0:
                        break
                    print('server data received')
                    c.sendall(data)
                    print('client data sent')
                    data = c.recv(1024)
                    if data is None or len(data) == 0:
                        break
                    print('client data received')
                    game_server_s.sendall(data)
                    print('server data sent')
                    if data.decode() == 'quit':
                        break
            except:
                pass
            finally:
                game_server_s.close()
                users.remove(a)
                game_servers.append(game_server)
                print('game finished')
    except Exception as e:
        raise e
    finally:
        c.close()


def print_users():
    while True:
        x = input()
        if x == 'users':
            print(len(users))


users_thread = threading.Thread(target=print_users)
users_thread.start()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', 8080))
s.listen(5)

while True:
    c, a = s.accept()
    t = threading.Thread(target=handle_request, args=(c, a))
    t.start()
