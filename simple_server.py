import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(('0.0.0.0', 8555))
server_socket.listen(1)
print("Server listening on port 8555")

while True:
    client_socket, addr = server_socket.accept()
    print(f"Connection from {addr}")
    client_socket.send(b'Hello from server\n')
    client_socket.close()
