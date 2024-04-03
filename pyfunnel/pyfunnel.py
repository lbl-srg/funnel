#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
# Python binding for funnel library: compareAndReport
#######################################################

from __future__ import absolute_import, division, print_function, unicode_literals

import argparse
import csv
import os
import textwrap
import sys

import core


if __name__ == "__main__":

    # Configure the argument parser.
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=(
            'Run funnel binary from terminal on two two-column CSV files.\n\n'
            'Output `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`, `test.csv` '
            'into the output directory (`./results` by default).'),
        epilog='Full documentation at https://github.com/lbl-srg/funnel'
    )
    required_named = parser.add_argument_group('required named arguments')

    required_named.add_argument(
        "--reference",
        help="Path of two-column CSV file with reference data",
        required=True
    )
    required_named.add_argument(
        "--test",
        help="Path of two-column CSV file with test data",
        required=True
    )
    parser.add_argument(
        "--output",
        help="Path of directory to store output data",
    )
    parser.add_argument(
        "--atolx",
        type=float,
        help="Absolute tolerance along x axis"
    )
    parser.add_argument(
        "--atoly",
        type=float,
        help="Absolute tolerance along y axis"
    )
    parser.add_argument(
        "--ltolx",
        type=float,
        help="Relative tolerance along x axis (relatively to the local value)"
    )
    parser.add_argument(
        "--ltoly",
        type=float,
        help="Relative tolerance along y axis (relatively to the local value)"
    )
    parser.add_argument(
        "--rtolx",
        type=float,
        help="Relative tolerance along x axis (relatively to the range)"
    )
    parser.add_argument(
        "--rtoly",
        type=float,
        help="Relative tolerance along y axis (relatively to the range)"
    )

    # Parse the arguments.
    args = parser.parse_args()

    # Check the arguments.
    assert os.path.isfile(args.reference),\
        "No such file: {}".format(args.reference)
    assert os.path.isfile(args.test),\
        "No such file: {}".format(args.test)

    # Extract data from files.
    data = dict()
    for s in ('reference', 'test'):
        data[s] = dict(x=[], y=[])
        with open(vars(args)[s]) as csvfile:
            spamreader = csv.reader(csvfile)
            if (len(next(spamreader)) != 2):
                raise RuntimeError("The {} CSV file must have exactly two columns.".format(s))
            for row in spamreader:
                try:
                    data[s]['x'].append(float(row[0]))
                    data[s]['y'].append(float(row[1]))
                except BaseException:
                    pass

    # Call the function.
    rc = core.compareAndReport(
        xReference=data['reference']['x'],
        yReference=data['reference']['y'],
        xTest=data['test']['x'],
        yTest=data['test']['y'],
        outputDirectory=args.output,
        atolx=args.atolx,
        atoly=args.atoly,
        ltolx=args.ltolx,
        ltoly=args.ltoly,
        rtolx=args.rtolx,
        rtoly=args.rtoly,
    )

    sys.exit(rc)
