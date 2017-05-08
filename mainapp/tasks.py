import dateutil.parser
import datetime
import requests
import time
import pandas as pd
import numpy as np

from io import StringIO

from users.models import *
from mainapp.models import *

from celery.task.schedules import crontab
from celery.decorators import periodic_task, task
from celery.utils.log import get_task_logger

logger = get_task_logger(__name__)

# Set Pandas display characteristics.                                           
import shutil
pd.set_option('display.width', shutil.get_terminal_size().columns)
pd.set_option('display.max_rows', 20) 
pd.set_option('display.max_columns', 18)
pd.set_option('display.max_colwidth', 60)

# Sparkfun data logger credentials
URL = 'https://data.sparkfun.com'
PUBLIC_KEY = 'YDdNyD6Wymf9ayKlrDR3'
PRIVATE_KEY = 'RnB7GnXWG6S0ZkAwDM8B'

@periodic_task(
    run_every=(crontab(minute='*/1')),
    name="sparky",
    ignore_result=True
)
def sparky():
    def download(url):
        last_time = time.gmtime(0)
        if len(Update.objects.all()) > 0:
            last_time = Update.objects.first().late_time_stamp
        response = requests.get(url, stream=True)
        if response.status_code != 200:
            return response
        df = None
        overflow = ''
        headers = []
        for chunk in response.iter_content(chunk_size=1024):
            if not chunk:
                continue
            chunk = chunk.decode("utf-8")
            chunk_lines = chunk.splitlines()
            df_chunk = pd.read_csv(StringIO('\n'.join(headers + [overflow + chunk_lines[0]] + chunk_lines[1:-1])))
            if len(headers) == 0:
                headers = [','.join(df_chunk.columns.values)]
            overflow = chunk_lines[-1]

            if df is None:
                df = df_chunk
            else:
                df = df.append(df_chunk)

            # print(df)
            try:
                if dateutil.parser.parse(df['timestamp'].max()).timetuple() <= last_time[:6]:
                    break
            except TypeError:
                return None

        df['timestamp'] = pd.to_datetime(df['timestamp'])
        df.set_index('timestamp', inplace=True)
        df = df.sort_index().sort_index(axis=1)
        return df

    df = None
    while not isinstance(df, pd.DataFrame):
        df = download('{}/output/{}.csv'.format(URL, PUBLIC_KEY))
        if not isinstance(df, pd.DataFrame):
            print('Going to sleep....zzzz')
            time.sleep(8)

    df.replace(to_replace='`',value=',',regex=True,inplace=True)

    def safe_eval(x):
        try:
            return eval(x)
        except TypeError:
            return x
    df = df.applymap(safe_eval)
    print(df)

    mappy = {}
    def expand(row):
        return {row.name + idx * np.timedelta64(30, 's'): x for idx, x in enumerate(list(zip(*row)))}
    def reduce_(row):
        mappy.update(row)
    df.apply(expand, axis=1).apply(reduce_)
    df = pd.DataFrame.from_items(mappy.items(), columns=df.columns, orient='index')
    df.sort_index(inplace=True)
    print(df)
