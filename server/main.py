from flask import Flask, request, flash, url_for, redirect, render_template, jsonify
from flask_restful import Api, Resource, reqparse, abort, fields, marshal_with
from flask_sqlalchemy import SQLAlchemy, BaseQuery
from sqlalchemy.orm import load_only
from flask_socketio import SocketIO, emit
import threading
import socket
import time
import json

HTTP_PORT = 5000
UDP_PORT = 5001
UDP_PORT_BROAD = 5050

communicating = 0

my_ip = socket.gethostbyname(socket.getfqdn())

ESP_ADR = ('192.168.0.10', 57683)


app = Flask(__name__)
api = Api(app)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database.db'
db = SQLAlchemy(app)

app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)



class onoff_thread (threading.Thread):
    def __init__(self):
        threading.Thread.__init__(self)

    def run(self):
        global communicating
        while True:
            while communicating:
                print("Esperando botao")
                time.sleep(1)
            communicating = 1
            try:

                ids = clients.query.order_by(clients.id).all()
                if ids:
                    print("Checando conexão")
                else:
                    print("Não há clientes para checar")
                for x in ids:

                    print("Checando ID:"+str(x.id))
                    cli = clients.query.filter_by(id=x.id).first()
                    if cli.function == "Ar":
                        cod = 32
                    elif cli.function == "Porta":
                        cod = 22
                    else:
                        cod = 1

                    print(cli.function+", código: "+str(cod))
                    msg = enviar_codigo(x.id, str(cod))
                    if msg == 4:
                        update_db_off(x.id)
                        print("Desconectado")
                    elif msg == 1:
                        update_db_ok(x.id, str(cod))
                        print("Conectado")
                    else:
                        update_db_value(x.id, str(cod), msg)
                        print("Conectado, valor:" +msg)
            except:
                print("Falha na checagem")
            communicating = 0
            time.sleep(10)   # Tempo entre cada check


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
        sock.bind(('', UDP_PORT_BROAD))
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
    ip = db.Column(db.String(20),)
    ip2 = db.Column(db.Integer,)
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

def __init__(self, name, ip, ip2, connected, function, value, action, act_value):
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
client_update_args.add_argument("name", type=str, help="Name of the client is required", required=True)
client_update_args.add_argument("connected", type=int, help="Views of the client is required", required=True)
client_update_args.add_argument("function", type=str, help="Views of the client is required", required=True)
client_update_args.add_argument("value", type=int, help="Likes on the client is required", required=True)
client_update_args.add_argument("ip", type=str, help="Likes on the client is required", required=True)
client_update_args.add_argument("ip2", type=int, help="Likes on the client is required", required=True)
client_update_args.add_argument("action", type=int, help="Likes on the client is required", required=True)
client_update_args.add_argument("act_value", type=int, help="Likes on the client is required", required=True)

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
        result.value = args['value']
        result.connected = args['connected']

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
    sock.bind(('', UDP_PORT))
    sock.settimeout(2)

    try:
        sent = sock.sendto(codigo.encode(), (esp.ip, int(esp.ip2)))
        data, address = sock.recvfrom(4096)
        print(data)
        data = str(data.decode('UTF-8'))
        if data == "OK":
            return 1
        elif data == "404":
            return 2
        else:
            return data
    except:
        print("Num achei nada")
        return 4




def enviar_codigo_temp(id,codigo,temp):
    esp = clients.query.filter_by(id=id).first()

    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP
    sock.bind(('', UDP_PORT))
    sock.settimeout(2)

    try:
        sent = sock.sendto((codigo + temp).encode(), (esp.ip, int(esp.ip2)))
        data, address = sock.recvfrom(4096)
        print(data)
        data = str(data.decode('UTF-8'))
        if data == "OK":
            return temp
        elif data == "404":
            return 2
        elif data == "tempErr":
            return 3
        else:
            return data

    except:
        print("Num achei nada")
        return 4


def update_db_ok(id,codigo):
    client = clients.query.filter_by(id=id).first()
    client.connected = 1
    if client.function == "Luz":
        print("Luz")
        if codigo == "10":
            print("10")
            client.action = 1
            db.session.commit()
        elif codigo == "11":
            print("11")
            client.action = 0
            db.session.commit()
    else:
        print("AT")
        if codigo == "31":
            print("31")
            client.action = 0
            db.session.commit()

    print("Salvo na DataBase")

def update_db_value(id, codigo, value):
    client = clients.query.filter_by(id=id).first()
    client.connected = 1
    if client.function == "Porta":
        print("Porta")
        if codigo == "22":
            print("22")
            client.value = int(value)
            db.session.commit()
    else:
        print("Ar")
        if codigo == "32":
            print("32")
            client.value = int(value)
            db.session.commit()
        elif codigo == "33":
            print("33")
            client.act_value = int(value)
            db.session.commit()
        elif codigo == "30":
            print("30")
            client.action = 1
            client.act_value = int(value)
            db.session.commit()
    print("Salvo na DataBase")

def update_db_off(id):
    client = clients.query.filter_by(id=id).first()
    client.connected = 0
    db.session.commit()


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
            global communicating
            while communicating:
                print("Esperando thread")
                time.sleep(0.1)
            communicating = 1
            if request.form['codigo'] == "30" or request.form['codigo'] == "33":
                print(request.form['codigo']+request.form['temp'])
                msg = enviar_codigo_temp(client_id,request.form['codigo'],request.form['temp'])

                # Salvar dados modificados na DB
            else:
                print(request.form['codigo'])
                msg = enviar_codigo(client_id, request.form['codigo'])

            if msg == 2:
                flash('404', 'error')
            elif msg == 3:
                flash('temperr', 'error')
            elif msg == 4:
                flash('Tente de novo.', 'error')
                update_db_off(client_id)
            elif msg == 1:
                update_db_ok(client_id,request.form['codigo'])
            else:
                update_db_value(client_id, request.form['codigo'],msg)
            communicating = 0

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


@app.route('/update/<int:client_id>/<int:part>', methods = ['GET'])
def update(client_id,part):
    client = clients.query.filter_by(id=client_id).first()
    if part == 1:
        return render_template('section_one.html', client=client)
    elif part == 2:
        return render_template('section_two.html', client=client)
    else:
        return jsonify(connected=client.connected)


@app.route('/update/index', methods = ['GET'])
def update_all():
    return render_template('section.html', clients = clients.query.order_by(clients.id).all())


@app.route('/update/new', methods = ['GET'])
def update_new():
    return render_template('section_new.html', newclients = new_clients.query.order_by(new_clients.id).all())

if __name__ == "__main__":
    thread = check_thread()
    thread.start()
    thread.pause()

    onoff = onoff_thread()
    onoff.start()

    app.run(host='0.0.0.0',port=HTTP_PORT)
