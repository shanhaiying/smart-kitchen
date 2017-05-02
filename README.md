# Kitchen Smart

### About
TODO

### Quickstart

- Create a copy of ``kitchen/settings/local.py.example`` in ``kitchen/settings/local.py``
- Create a ``.env`` file in the root of the project and add ``DJANGO_SETTINGS_MODULE="kitchen.settings.local"`` to it
- Create the migrations for `users` app: `python manage.py makemigrations`
- Run the migrations: `python manage.py migrate`

### Tools

- Setup [editorconfig](http://editorconfig.org/), [flake8](http://flake8.pycqa.org/en/latest/) and [ESLint](http://eslint.org/) in the text editor you will use to develop.

### Running the project

- `pip install -r requirements.txt`
- `npm install`
- `make bundle`
- `python manage.py runserver`
- `python manage.py celery worker --beat --loglevel=info`

### Testing

`make test`

Will run django tests using `--keepdb` and `--parallel`. You may pass a path to the desired test module in the make command. E.g.:

`make test someapp.tests.test_views`

## Checking lint

- Manually with `flake8` and `npm run lint` on project root.
- During development with an editor compatible with flake8 and ESLint.

## Pre-commit hooks

- Run `pre-commit install` to enable the hook into your git repo. The hook will run automatically for each commit.
- Run `git commit -m "Your message" -n` to skip the hook if you need.
