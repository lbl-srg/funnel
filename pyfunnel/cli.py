#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""Standalone command-line interface for the funnel C library.

This script provides a direct interface to the 'compareAndReport' function of the funnel
library through the pyfunnel Python bindings. It can be executed standalone
without installing the pyfunnel package, or run as part of an installed package.
"""

import argparse
import csv
import os
import sys
from pathlib import Path

# Add current directory to path if needed (CLI script run without package install)
if __name__ == "__main__":
    current_dir = Path(__file__).parent
    if str(current_dir) not in sys.path:
        sys.path.insert(0, str(current_dir))

from pyfunnel import compareAndReport


def main():
    """Main entry point for the pyfunnel command-line interface."""

    # Configure the argument parser.
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=(
            'Run funnel binary from terminal on two two-column CSV files.\n\n'
            'The tool computes the deviation of test data beyond the funnel envelope\n'
            'generated around reference data.\n'
            'It outputs `errors.csv`, `lowerBound.csv`, `upperBound.csv`,\n'
            '`reference.csv`, `test.csv` into the output directory (`./results` by default).\n\n'
            'Each CSV file must contain two columns: x (independent variable, e.g., time)\n'
            'and y (dependent variable, e.g., temperature, pressure). The funnel boundaries\n'
            'are computed based on tolerances that can be set independently for x and y.\n\n'
            'Tolerance can be specified in three forms for each variable:\n'
            '  - Absolute tolerance\n'
            '  - Relative to variable value\n'
            '  - Relative to variable range\n'
            'If not set, the default tolerance is zero. For each direction, the funnel\n'
            'is sized based on the maximum of the three tolerance calculations.\n\n'
            'Examples:\n'
            '  # If pyfunnel package is installed:\n'
            '  funnel --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002\n'
            '  # If package is not installed (run from the funnel source directory):\n'
            '  python pyfunnel/cli.py --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002'
        ),
        epilog='Full documentation at https://github.com/lbl-srg/funnel',
    )
    required_named = parser.add_argument_group('required named arguments')

    required_named.add_argument(
        '--reference', help='Path of two-column CSV file with reference data', required=True
    )
    required_named.add_argument(
        '--test', help='Path of two-column CSV file with test data', required=True
    )
    parser.add_argument(
        '--output',
        help='Path of directory to store output data',
    )
    parser.add_argument('--atolx', type=float, help='Absolute tolerance along x axis')
    parser.add_argument('--atoly', type=float, help='Absolute tolerance along y axis')
    parser.add_argument(
        '--ltolx',
        type=float,
        help='Relative tolerance along x axis (relatively to the local value)',
    )
    parser.add_argument(
        '--ltoly',
        type=float,
        help='Relative tolerance along y axis (relatively to the local value)',
    )
    parser.add_argument(
        '--rtolx', type=float, help='Relative tolerance along x axis (relatively to the range)'
    )
    parser.add_argument(
        '--rtoly', type=float, help='Relative tolerance along y axis (relatively to the range)'
    )

    # Parse the arguments.
    args = parser.parse_args()

    # Check the arguments.
    assert os.path.isfile(args.reference), 'No such file: {}'.format(args.reference)
    assert os.path.isfile(args.test), 'No such file: {}'.format(args.test)

    # Extract data from files.
    data = dict()
    for s in ('reference', 'test'):
        data[s] = dict(x=[], y=[])
        with open(vars(args)[s]) as csvfile:
            spamreader = csv.reader(csvfile)
            for i, row in enumerate(spamreader):
                if (l := len(row)) != 2:
                    raise IOError(
                        'The {} CSV file must have exactly two columns. Row {} contains {} elements.'.format(
                            s, i, l
                        )
                    )
                try:
                    data[s]['x'].append(float(row[0]))
                    data[s]['y'].append(float(row[1]))
                except BaseException:
                    pass

    # Call the function.
    rc = compareAndReport(
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


if __name__ == '__main__':
    main()
