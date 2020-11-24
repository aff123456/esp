from flask import Flask, request, flash, url_for, redirect, render_template
from flask_restful import Api, Resource, reqparse, abort, fields, marshal_with
from flask_sqlalchemy import SQLAlchemy
from flask_socketio import SocketIO, emit
from threading import Thread
import socket
import time

HTTP_PORT = 5000
UDP_PORT = 5001

ESP_ADR = ('192.168.0.121', 5001)


app = Flask(__name__)
api = Api(app)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///database.db'
db = SQLAlchemy(app)

app.config['SECRET_KEY'] = 'secret!'
socketio = SocketIO(app)


def check_thread():
    sock = socket.socket(socket.AF_INET,  # Internet
                         socket.SOCK_DGRAM)  # UDP
    sock.bind(('', UDP_PORT))

    response = 'pfg_ip_response_serv'

    while True:
        data, address = sock.recvfrom(4096)
        data = str(data.decode('UTF-8'))
        if data == 'pfg_ip_broadcast_cl':
            print('Client ip: ' + str(address[0]))
            print(address)
            print('responding...')
            sent = sock.sendto(response.encode(), address)
            print('Sent confirmation back')



class VideoModel(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(100), nullable=False)
    views = db.Column(db.Integer, nullable=False)
    likes = db.Column(db.Integer, nullable=False)

    def __repr__(self):
        return f"Video(name={name}, views={views}, likes={likes})"

#db.create_all()


video_put_args = reqparse.RequestParser()
video_put_args.add_argument("name", type=str, help="Name of the video is required", required=True)
video_put_args.add_argument("views", type=int, help="Views of the video is required", required=True)
video_put_args.add_argument("likes", type=int, help="Likes on the video is required", required=True)

video_update_args = reqparse.RequestParser()
video_update_args.add_argument("name", type=str)
video_update_args.add_argument("views", type=int)
video_update_args.add_argument("likes", type=int)

resource_fields = {
    'id': fields.Integer,
    'name': fields.String,
    'views': fields.Integer,
    'likes': fields.Integer
}


class Video(Resource):
    @marshal_with(resource_fields)
    def get(self, video_id):
        result = VideoModel.query.filter_by(id=video_id).first()
        if not result:
            abort(404, message="Could not find video...")
        return result

    @marshal_with(resource_fields)
    def put(self, video_id):
        args = video_put_args.parse_args()
        result = VideoModel.query.filter_by(id=video_id).first()
        if result:
            abort(409, message="Video id taken...")
        video = VideoModel(id=video_id, name=args['name'], views=args['views'], likes=args['likes'])
        db.session.add(video)
        db.session.commit()
        return video, 201

    @marshal_with(resource_fields)
    def patch(self, video_id):
        args = video_update_args.parse_args()
        result = VideoModel.query.filter_by(id=video_id).first()
        if not result:
            abort(404, message="Video doesnt exist, cannot update")

        if args['name']:
            result.name = args['name']
        if args['views']:
            result.views = args['views']
        if args['likes']:
            result.likes = args['likes']

        db.session.commit()
        return result

    def delete(self, video_id):
        abort_if_video_id_doesnt_exist(video_id)
        del videos[video_id]
        return '',204



class Videos(Resource):
    @marshal_with(resource_fields)
    def get(self):
        result = VideoModel.query.order_by(VideoModel.id).all()
        if not result:
            abort(404, message="Could not find videos...")
        return result

api.add_resource(Video, "/video/<int:video_id>")
api.add_resource(Videos, "/videos/")

@socketio.on('message')
def handle_message(message):
    print('received message: ' + message)


@app.route("/")
def index():
  return render_template('index.html')

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
    return render_template('show_all.html', videos = VideoModel.query.order_by(VideoModel.id).all() )

@app.route('/show_one/<int:video_id>', methods = ['GET', 'POST'])
def show_one(video_id):
    if request.method == 'POST':
        if not request.form['name']:
            flash('Please enter a new name', 'error')
        else:
            result = VideoModel.query.filter_by(id=video_id).first()

            result.name = request.form['name']

            db.session.commit()

            flash('Record was successfully added')
    return render_template('show_one.html', video = VideoModel.query.filter_by(id=video_id).first())

if __name__ == "__main__":

    thread = Thread(target=check_thread)
    thread.daemon = True
    thread.start()
    print('UDP Socket running in thread:', thread.getName())

    app.run(host='0.0.0.0',port=HTTP_PORT)
