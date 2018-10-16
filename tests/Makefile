
all:
	(cd fail1 && \
	 ../../bin/funnel -b trended.csv -c simulated.csv -o results/ && \
	 cd results && ../../makePlot.py --reference_csv refData.csv --test_csv testData.csv --lower_csv lowerData.csv --upper_csv upperData.csv)

clean:
	rm -rf fail1/results
