test:
	pytest tests

coverage:
	pytest --cov=my_own_bricks --cov-report term-missing -vv

docstring_coverage:
	interrogate -v my_own_bricks/ -e my_own_bricks/__init__.py --badge-style flat --generate-badge .
