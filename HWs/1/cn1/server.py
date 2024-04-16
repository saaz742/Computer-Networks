import socket
import random


def check_corr(board):
    if 0 <= int(board) and int(board) <= 2:
        return True
    else:
        return False


load_balancer_address = 'localhost'
load_balancer_port = 8080
game_server_port = 8090


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind(('', game_server_port))
s.listen(5)


tmp_s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
tmp_s.connect((load_balancer_address, load_balancer_port))
tmp_s.sendall(f'server {game_server_port}'.encode())
tmp_s.close()


while True:
    c, a = s.accept()
    print('game started')
    board = [['-'] * 3 for _ in range(3)]
    c.sendall('\n'.join([''.join(row) for row in board]).encode())
    print('board sent')
    try:
        while True:
            data = c.recv(1024)
            print('data received')
            move = data.decode().split()
            if check_corr(int(move[0])) and check_corr(int(move[1])):
                if (board[int(move[0])][int(move[1])] == '-'):
                    board[int(move[0])][int(move[1])] = 'X'
                    if ''.join(board[0]) == 'XXX' or ''.join(board[1]) == 'XXX' or ''.join(board[2]) == 'XXX' or ''.join(board[0][0] + board[1][0] + board[2][0]) == 'XXX' or ''.join(board[0][1] + board[1][1] + board[2][1]) == 'XXX' or ''.join(board[0][2] + board[1][2] + board[2][2]) == 'XXX' or ''.join(board[0][0] + board[1][1] + board[2][2]) == 'XXX' or ''.join(board[0][2] + board[1][1] + board[2][0]) == 'XXX':
                        c.sendall(b'You won!')
                        break
                    if [x for row in board for x in row].count('-') == 0:
                        c.sendall(b'Draw!')
                        break
                    while True:
                        x = random.randint(0, 2)
                        y = random.randint(0, 2)
                        if board[x][y] == '-':
                            break
                    board[x][y] = 'O'
                    if ''.join(board[0]) == 'OOO' or ''.join(board[1]) == 'OOO' or ''.join(board[2]) == 'OOO' or ''.join(board[0][0] + board[1][0] + board[2][0]) == 'OOO' or ''.join(board[0][1] + board[1][1] + board[2][1]) == 'OOO' or ''.join(board[0][2] + board[1][2] + board[2][2]) == 'OOO' or ''.join(board[0][0] + board[1][1] + board[2][2]) == 'OOO' or ''.join(board[0][2] + board[1][1] + board[2][0]) == 'OOO':
                        c.sendall(b'You lost!')
                        break
                    if [x for row in board for x in row].count('-') == 0:
                        c.sendall(b'Draw!')
                        break
                    print('one turn done')
                    c.sendall('\n'.join([''.join(row)
                              for row in board]).encode())
                    print('board sent')
                else:
                    board = board
                    c.sendall(b'Invalid move!')
            else:
                board = board
                c.sendall(b'Wrong corrdination!')
    except Exception as e:
        raise e
    finally:
        c.close()
