from flask import Flask, Blueprint
from os import getenv

from backend.models import db, ma
from backend.routes import general, stock_data_api, stock_info_api
from backend.schedulers import UpdateScheduler

# Database settings
postgres = {
  'username': 'postgres',
  'password': 'draco',
  'host': '35.188.115.222',
  'port': '5432',
  'database': 'postgres',
  'instance': 'draco-web08:us-central1:postgres'
}

# Initialize Flask
app = Flask(__name__, template_folder='static')
app.url_map.strict_slashes = False

# Connect to the database
if getenv('GOOGLE_CLOUD_PROJECT', '') is not '':
  # For connecting once deployed
  app.config['SQLALCHEMY_DATABASE_URI'] = ('postgresql+psycopg2://{username}:'
    '{password}@/{database}?host=/cloudsql/{instance}').format(**postgres)
else:
  # For connecting locally
  app.config['SQLALCHEMY_DATABASE_URI'] = ('postgresql+psycopg2://{username}:'
    '{password}@{host}:{port}/{database}').format(**postgres)

app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

# Initialize SQLAlchemy and Marshmallow to be used with this app
db.init_app(app)
ma.init_app(app)

# Register the blueprints
app.register_blueprint(general)
app.register_blueprint(stock_data_api)
app.register_blueprint(stock_info_api)

# Create the update scheduler if not deployed
if getenv('GOOGLE_CLOUD_PROJECT', '') is not '':
  update_scheduler = UpdateScheduler()
  update_scheduler.create(app)
