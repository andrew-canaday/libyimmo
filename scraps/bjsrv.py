import bjoern
from my import app

bjoern.run(
    wsgi_app=app,
    host='0.0.0.0',
    port=8081,
    reuse_port=True
)
