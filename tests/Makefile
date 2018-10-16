DIRS=fail1  success1
TEST=echo "Running test in `pwd`" && ../../bin/funnel -b trended.csv -c simulated.csv -o results/
PLOT=cd results && ../../makePlot.py --reference_csv refData.csv --test_csv testData.csv --lower_csv lowerData.csv --upper_csv upperData.csv

all:
	for di in $(DIRS); \
	do \
		(cd $$di; $(TEST) && $(PLOT) )\
	done

clean:
	for di in $(DIRS); \
	do \
	rm -rf $$di/results; \
	done
