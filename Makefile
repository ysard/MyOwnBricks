test:
	pytest tests

coverage:
	pytest --cov=python_poc --cov-report term-missing -vv

docstring_coverage:
	interrogate -v python_poc/ -e python_poc/__init__.py --badge-style flat --generate-badge .
