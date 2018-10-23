#!/usr/bin/env python
#######################################################
# Script that runs the funnel software with a python
# binding.
#######################################################

if __name__ == "__main__":
    import sys
    # Add directory where pyfunnel.py is to search path
    sys.path.append('../bin')

    import pyfunnel as pf

    nReference = 6
    nTest = 6
    tReference = range(0, nReference)
    yReference = [0., 0., 1., 1., 0., 0.]
    tTest = range(0, nTest)
    yTest = [0, 0, 1.1, 1, 0, 0]
    outputDirectory = 'testPython/'
    tolerance = 0.002
    axes = 'Y'
    useRelativeTolerance = True

    pf.compareAndReport(
        tReference,
        yReference,
        tTest,
        yTest,
        outputDirectory,
        tolerance,
        axes,
        useRelativeTolerance)
