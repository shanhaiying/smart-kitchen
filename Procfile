web: gunicorn kitchen.wsgi --limit-request-line 8188 --log-file -
main_worker: python manage.py celery worker --beat --loglevel=info --logfile=logs/celery.log
