DIRS=fail1 success1
TEST=echo "Running test in `pwd`" && ../../bin/funnel -b trended.csv -c simulated.csv -o results/
#PLOT=cd results && ../../makePlot.py --reference_csv reference.csv --test_csv test.csv --lower_csv lowerBound.csv --upper_csv upperBound.csv

all: test-c test-python

test-c:
	@for di in $(DIRS); \
	do \
		(cd $$di; $(TEST) )\
	done

test-python:
	python pythonAPI.py

clean:
	@for di in $(DIRS); \
	do \
	rm -rf $$di/results; \
	done
	rm -rf testPython
