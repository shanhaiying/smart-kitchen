from flask import Flask, request, Response
import requests

app = Flask(__name__)

DESTINATION_URL = 'https://kitchen-smart.herokuapp.com'

@app.route('/', defaults={'path': ''}, methods=['GET', 'POST'])
@app.route('/<path:path>', methods=['GET', 'POST'])
def reverse_proxy(path):
    request_kwargs = {
        'method': request.method,
        'url': '{}/{}'.format(DESTINATION_URL, path),
        'headers': {key: value for (key, value) in request.headers if key != 'Host'},
        'data': request.get_data(),
        'params': request.args,
        'cookies': request.cookies,
        'allow_redirects': False
    }

    print('Reverse proxy for {}'.format(DESTINATION_URL))
    print(request_kwargs)

    resp = requests.request(**request_kwargs)

    excluded_headers = ['content-encoding', 'content-length', 'transfer-encoding', 'connection']
    headers = [(name, value) for (name, value) in resp.raw.headers.items()
               if name.lower() not in excluded_headers]

    response = Response(resp.content, resp.status_code, headers)
    return response

if __name__ == '__main__':
    app.run()
