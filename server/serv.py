import socket
import socketserver
import http.server
import threading


class ServerHandler(http.server.SimpleHTTPRequestHandler):

    def do_POST(self):
      content_len = int(self.headers.getheader('content-length', 0))
      post_body = self.rfile.read(content_len)
      print(post_body)

server_address = ('', 5001)

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind(server_address)

response = 'pfg_ip_response_serv'

Handler = http.server.SimpleHTTPRequestHandler
t = threading.Thread(target=socketserver.TCPServer(server_address, Handler).serve_forever)
t.setDaemon(True)  # don't hang on exit
t.start()
print('Server loop running in thread:', t.getName())

while True:

	data, address = sock.recvfrom(4096)
	data = str(data.decode('UTF-8'))
	print('Received ' + str(len(data)) + ' bytes from ' + str(address) )
	print('Data:' + data)

	if data == 'pfg_ip_broadcast_cl':
		print('responding...')
		sent = sock.sendto(response.encode(), address)
		print('Sent confirmation back')