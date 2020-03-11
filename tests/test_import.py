#!/usr/bin/env python
# -*- coding: utf-8 -*-
from __future__ import absolute_import, division, print_function, unicode_literals

import os, sys
import re
import json
import subprocess
import pandas as pd

pyfunnel_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir, 'funnel'))
sys.path.append(pyfunnel_dir)
import pyfunnel as pf


def read_res(test_dir):
    tmp = pd.read_csv(os.path.join(test_dir, 'results', 'errors.csv'))

    return tmp


def test_log(test_name, test_dir, tmp_dir, dif_err):
    test_log = dict(
        test_name=test_name,
        test_dir=test_dir,
        tmp_dir=tmp_dir,
        dif_err=dif_err
    )
    with open(os.path.join(tmp_dir, 'test_log.json'), 'w') as f:
        json.dump(test_log, f)


def dif_test(test_dir):
    """Assess the differences between the current test and the reference test.
    The differences are assessed for x and y values of error.csv files between
    the test run in the current directory (file ./results/error.csv) and
    the reference test run in the directory passed as argument
    (file test_dir/results/error.csv).

    Args:
        test_dir (str): path of reference test directory

    Returns:
        list (len = 2): number of differences along x and y
    """
    test_ref = read_res(test_dir)
    test_new = read_res('./')
    nb_dif_x = sum(test_ref.iloc(axis=1)[0] != test_new.iloc(axis=1)[0]) / len(test_ref)
    nb_dif_y = sum(test_ref.iloc(axis=1)[1] != test_new.iloc(axis=1)[1]) / len(test_ref)

    return [nb_dif_x, nb_dif_y]


def run_pyfunnel(test_dir):
    """Run pyfunnel compareAndReport function.
    The test is run:
    *   with the parameters, reference and test values from the test directory
        passed as argument;
    *   from current directory (to which output directory path is relative).

    Args:
        test_dir (str): path of test directory

    Returns:
        int: exit status of funnel binary
    """

    with open(os.path.join(test_dir, 'param.json'), 'r') as f:
        par = json.load(f)

    ref = pd.read_csv(os.path.join(test_dir, par['reference']))
    test = pd.read_csv(os.path.join(test_dir, par['test']))
    par['outputDirectory'] = par['output']

    for t in ['atolx', 'atoly', 'rtolx', 'rtoly']:
        try:
            par[t]
        except KeyError:
            par[t] = None

    rc = pf.compareAndReport(
        ref.iloc(axis=1)[0],
        ref.iloc(axis=1)[1],
        test.iloc(axis=1)[0],
        test.iloc(axis=1)[1],
        **{k: par[k] for k in ['outputDirectory', 'atolx', 'atoly', 'rtolx', 'rtoly']}
    )

    return rc


if __name__ == "__main__":
    pass