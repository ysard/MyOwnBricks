test:
	pytest tests -vv

coverage:
	pytest --cov=my_own_bricks --cov-report term-missing -vv

docstring_coverage:
	interrogate -v my_own_bricks/ -e my_own_bricks/__init__.py --badge-style flat --generate-badge ./images/

coverage_badge:
	coverage-badge -f -o ./images/coverage.svg

release:
ifndef VERSION
	@echo Warning: VERSION variable isn\'t defined\; continue? [Y/n]
	@read line; if [ $$line = "n" ]; then echo aborting; exit 1 ; fi
endif
	@echo "Releasing $$VERSION..."
	sed -i "s/^version=.*/version=$$VERSION/g" library.properties
	git add library.properties
	git commit -m "Bump $$VERSION"
	git tag $$VERSION
	@echo "Done, just make a git push!"
