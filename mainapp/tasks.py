import dateutil.parser
import requests
import pandas as pd

from io import StringIO

from celery.task.schedules import crontab
from celery.decorators import periodic_task, task
from celery.utils.log import get_task_logger

logger = get_task_logger(__name__)

# Sparkfun data logger credentials
URL = 'https://data.sparkfun.com'
# PUBLIC_KEY = 'YDdNyD6Wymf9ayKlrDR3'
PUBLIC_KEY = '9J2Zxy4gyjFWvZn7qO60'
PRIVATE_KEY = 'RnB7GnXWG6S0ZkAwDM8B'

# FORMAT
# uid | s0 .. s9 | batt | timestamp


@periodic_task(
    run_every=(crontab(minute='*/1')),
    name="sparky",
    ignore_result=True
)
def sparky():
    return # DO NOTHING FOR NOW
    def download(url):
        response = requests.get(url, stream=True)
        print(response.status_code)
        for chunk in response.iter_content(chunk_size=1024):
            if not chunk:
                continue
            chunk = chunk.decode("utf-8")
            df = pd.read_csv(StringIO('\n'.join(chunk.splitlines()[:-1])))
            print(df)

    # dateutil.parser.parse(timestamp)

    download('{}/output/{}.csv'.format(URL, PUBLIC_KEY))
    # 2017-05-01T17:31:09.829Z