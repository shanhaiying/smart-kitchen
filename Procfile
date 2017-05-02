web: gunicorn kitchen.wsgi --limit-request-line 8188 --log-file -
worker: celery worker --app=kitchen --loglevel=info
