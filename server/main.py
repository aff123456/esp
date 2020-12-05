from flask import Flask, request, flash, url_for, redirect, render_template
from flask_restful import Api, Resource, reqparse, abort, fields, marshal_with
from flask_sqlalchemy import SQLAlchemy
from flask_socketio import SocketIO, emit
import threading
import socket
import time
import json

HTTP_PORT = 5001
UDP_PORT = 5005

my_ip = socket.gethostbyname(socket.getfqdn())

ESP_ADR = ('192.168.0.10', 57683)


app = Flask(__name__)
api = Api(app)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database.db'
db = SQLAlchemy(app)

app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)


class check_thread(threading.Thread):

    def __init__(self, *args, **kwargs):
        super(check_thread, self).__init__(*args, **kwargs)
        self.__flag = threading.Event() # The flag used to pause the thread
        self.__flag.set() # Set to True
        self.__running = threading.Event() # Used to stop the thread identification
        self.__running.set() # Set running to True



    def run(self):

        print('UDP Socket running in thread')

        sock = socket.socket(socket.AF_INET,  # Internet
                             socket.SOCK_DGRAM)  # UDP
        sock.bind(('', UDP_PORT))
        response = 'pfg_ip_response_serv'
        sock.settimeout(10)

        while self.__running.isSet():
            self.__flag.wait() # return immediately when it is True, block until the internal flag is True when it is False
            print("Bora")

            try:
                data, address = sock.recvfrom(4096)
                print(data)
                data = str(data.decode('UTF-8'))
                print(data)
                data_json = json.loads(data)
                print(data_json["id"])

                if data_json["id"]:
                    print('Client ip: ' + str(address[0]))
                    print(address)
                    print('responding...')
                    sent = sock.sendto(response.encode(), address)
                    print('Sent confirmation back')
                    result = new_clients.query.filter_by(id=data_json['id']).first()
                    if not result:
                        client = new_clients(id=data_json["id"],ip = str(address[0]), ip2 = address[1], name=data_json["name"],\
                             function=data_json["function"])
                        try:
                            db.session.add(client)
                            db.session.commit()
                        except:
                            print("An exception occurred")
                    else:
                        print('Client already on the list')
            except:
                print("Num achei nada")

            if not self.__flag.isSet():
                print('UDP Socket stopped searching')


    def pause(self):
        self.__flag.clear() # Set to False to block the thread

    def resume(self):
        self.__flag.set() # Set to True, let the thread stop blocking
        print('UDP Socket is searching')

    def stop(self):
        self.__flag.set() # Resume the thread from the suspended state, if it is already suspended
        self.__running.clear() # Set to False

    def get_flag(self):
        return self.__flag.isSet()





class clients(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    ip = db.Column(db.String(20), primary_key=True)
    ip2 = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100))
    connected = db.Column(db.Boolean)
    function = db.Column(db.String(100))
    value = db.Column(db.Integer)
    action = db.Column(db.Boolean)
    act_value = db.Column(db.Integer)


class new_clients(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    ip = db.Column(db.String(20), primary_key=True)
    ip2 = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100))
    function = db.Column(db.String(100))

def __init__(self, name, ip, ip2, connected, function,value,action,act_value):
   self.name = name
   self.ip = ip
   self.ip2 = ip2
   self.connected = connected
   self.function = function
   self.value = value
   self.action = action
   self.act_value = act_value

db.create_all()


client_put_args = reqparse.RequestParser()
client_put_args.add_argument("name", type=str, help="Name of the client is required", required=True)
client_put_args.add_argument("connected", type=int, help="Views of the client is required", required=True)
client_put_args.add_argument("function", type=str, help="Views of the client is required", required=True)
client_put_args.add_argument("value", type=int, help="Likes on the client is required", required=True)
client_put_args.add_argument("ip", type=str, help="Likes on the client is required", required=True)
client_put_args.add_argument("ip2", type=int, help="Likes on the client is required", required=True)
client_put_args.add_argument("action", type=int, help="Likes on the client is required", required=True)
client_put_args.add_argument("act_value", type=int, help="Likes on the client is required", required=True)


client_update_args = reqparse.RequestParser()
client_update_args.add_argument("name", type=str)
client_update_args.add_argument("connected", type=bool)
client_update_args.add_argument("function", type=str)
client_update_args.add_argument("value", type=int)

resource_fields = {
    'id': fields.Integer,
    'name': fields.String,
    'connected': fields.Integer,
    'function': fields.String,
    'value': fields.Integer,
    'ip': fields.String,
    'ip2': fields.Integer
}


class Client(Resource):
    @marshal_with(resource_fields)
    def get(self, client_id):
        result = clients.query.filter_by(id=client_id).first()
        if not result:
            abort(404, message="Could not find client...")
        return result

    @marshal_with(resource_fields)
    def put(self, client_id):
        args = client_put_args.parse_args()
        result = clients.query.filter_by(id=client_id).first()
        if result:
            abort(409, message="Client id taken...")
        client = clients(id=client_id, name=args['name'], ip=args['ip'], ip2=args['ip2'], connected=args['connected'],\
                         function=args['function'], value=args['value'], action=args['action'], act_value=args['act_value'])
        db.session.add(client)
        db.session.commit()
        return client, 201

    @marshal_with(resource_fields)
    def patch(self, client_id):
        args = client_update_args.parse_args()
        result = clients.query.filter_by(id=client_id).first()
        if not result:
            abort(404, message="client doesnt exist, cannot update")

        if args['name']:
            result.name = args['name']
        if args['connected']:
            result.connected = args['connected']
        if args['function']:
            result.likes = args['function']
        if args['value']:
            result.likes = args['value']

        db.session.commit()
        return result

#    def delete(self, client_id):
#        abort_if_client_id_doesnt_exist(client_id)
#        del clients[client_id]
#        return '',204


def enviar_codigo(id,codigo):
    esp = clients.query.filter_by(id=id).first()

    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP

    sent = sock.sendto(codigo.encode(), (esp.ip, int(esp.ip2)))
    # Ouvir a confirmação do código
    # Salvar dados modificados na DB


class Clients(Resource):
    @marshal_with(resource_fields)
    def get(self):
        result = clients.query.order_by(clients.id).all()
        if not result:
            abort(404, message="Could not find videos...")
        return result

api.add_resource(Client, "/client/<int:client_id>")
api.add_resource(Clients, "/clients/")

@socketio.on('message')
def handle_message(message):
    print('received message: ' + message)


@app.route("/")
def index():
  return render_template('index.html', clients = clients.query.order_by(clients.id).all() )

@app.route('/show_all', methods = ['GET', 'POST'])
def show_all():
    if request.method == 'POST':
        if not request.form['codigo']:
            flash('Please enter a code', 'error')
        else:

            codigo = request.form['codigo']
            sock = socket.socket(socket.AF_INET,  # Internet
                                 socket.SOCK_DGRAM)  # UDP

            sent = sock.sendto(codigo.encode(), ESP_ADR)

            flash('codigo enviado')
    return render_template('show_all.html', clients = clients.query.order_by(clients.id).all() )

@app.route('/show_one/<int:client_id>', methods = ['GET', 'POST'])
def show_one(client_id):
    if request.method == 'POST':
        if request.form['button'] == "Edit":
            if not request.form['name']:
                flash('Digite um novo nome.', 'error')
            else:
                result = clients.query.filter_by(id=client_id).first()
                result.name = request.form['name']
                db.session.commit()
                flash('Nome foi alterado.')
        elif request.form['button'] == "Delete Client":
            db.session.delete(clients.query.filter_by(id=client_id).first())
            db.session.commit()
            return redirect(url_for('index'))
        elif request.form['button'] == "funcao":
            if request.form['codigo'] == "30" or request.form['codigo'] == "33":
                print(request.form['codigo']+request.form['temp'])
                #enviar_codigo(client_id,request.form['codigo']+request.form['temp'])
            else:
                print(request.form['codigo'])
                #enviar_codigo(client_id, request.form['codigo'])
        else:
            if not request.form['codigo']:
                flash('Digite um código.', 'error')
            else:

                codigo = request.form['codigo']
                sock = socket.socket(socket.AF_INET,  # Internet
                                     socket.SOCK_DGRAM)  # UDP

                sent = sock.sendto(codigo.encode(), (request.form['ip'], int(request.form['ip2'])))

                flash('Código enviado.')
    return render_template('show_one.html', client = clients.query.filter_by(id=client_id).first(), clients = clients.query.order_by(clients.id).all())

@app.route('/new_client/', methods = ['GET', 'POST'])
def new_client():
    if request.method == 'POST':
        if request.form['button'] == "Search":
            thread.resume()
            #flash('Procurando.')
        elif request.form['button'] == "Stop Search":

            thread.pause()
            #flash('Parou de procurar.')

        elif request.form['button'] == "Clear List":
            db.session.query(new_clients).delete()
            db.session.commit()
            flash('Lista foi limpa.')
        else:
            result = clients.query.filter_by(id=request.form['id']).first()
            if not result:
                client = clients(id=request.form['id'], ip=request.form['ip'], ip2=request.form['ip2'], name=request.form['name'], connected=int(request.form['connected']),
                             function=request.form['function'], value=request.form['value'], action=int(request.form['action']), act_value=request.form['act_value'])
                db.session.add(client)
                db.session.commit()

                #db.session.query(new_clients).delete()
                db.session.delete(new_clients.query.filter_by(id=request.form['id']).first())
                db.session.commit()

                flash('Cliente foi adicionado.')

            else:
                flash('Id já existente.')


            thread.pause()

            #return redirect(url_for('show_all'))
    return render_template('new_client.html', newclients=new_clients.query.order_by(new_clients.id).all() , clients = clients.query.order_by(clients.id).all(), searching = thread.get_flag())



if __name__ == "__main__":
    thread = check_thread()

    thread.start()
    thread.pause()
    app.run(host='0.0.0.0',port=HTTP_PORT)
