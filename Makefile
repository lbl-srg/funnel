DIR=pyfunnel

PEP8_ARGS=--recursive --max-line-length=100 \
  --ignore="E402" \
  --aggressive --aggressive --aggressive \
  pyfunnel


clean:
	rm -rf build
	rm -rf *.egg-info
	rm -rf dist


dist:	clean pep8
	python setup.py sdist bdist_wheel
	rm -rf build
	rm -rf pyfunnel.egg-info
	twine check dist/*
	@echo "Source distribution is in directory dist"
	@echo "To upload to https://test.pypi.org, run 'make upload-test'"
	@echo "To upload to https://pypi.org, run 'make upload'"


pep8:
ifeq ($(PEP8_CORRECT_CODE), true)
	@echo "*** Running autopep8 to correct code"
	autopep8 --in-place $(PEP8_ARGS)
	@echo "*** Checking for required code changes (apply with 'make pep8 PEP8_CORRECT_CODE=true')"
	git diff --exit-code .
else
	@echo "*** Checking for required code changes (apply with 'make pep8 PEP8_CORRECT_CODE=true')"
	autopep8 --diff $(PEP8_ARGS)
endif


upload:
	twine upload --verbose --repository pypi dist/*


upload-test:
	twine upload --verbose --repository testpypi dist/*
