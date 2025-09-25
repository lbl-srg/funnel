#!/usr/bin/env python
# -*- coding: utf-8 -*-

import json
import os
import re
import shutil
import subprocess
import sys

import numpy as np
import pandas as pd

pyfunnel_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), os.path.pardir))
# Another version of pyfunnel may be installed locally.
# The following ensures that the current development version is used.
sys.path.insert(1, pyfunnel_dir)
import pyfunnel


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


def dif_test(test_dir, rtol=1e-12):
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
    dif_x = np.logical_not(np.isclose(test_ref.iloc(axis=1)[0], test_new.iloc(axis=1)[0], rtol=rtol))
    dif_y = np.logical_not(np.isclose(test_ref.iloc(axis=1)[1], test_new.iloc(axis=1)[1], rtol=rtol))
    frac_dif_x = sum(dif_x) / len(test_ref)
    frac_dif_y = sum(dif_y) / len(test_ref)
    pd.options.display.float_format = '{:.16g}'.format
    if frac_dif_x > 0:
        print('*** x values that differ between reference and test results:')
        print(pd.concat([
            test_ref[dif_x].rename(lambda x: x + '_ref', axis=1),
            test_new[dif_x].rename(lambda x: x + '_test', axis=1)], axis=1))
    if frac_dif_y > 0:
        print('*** y values that differ between reference and test results:')
        print(pd.concat([
            test_ref[dif_y].rename(lambda x: x + '_ref', axis=1),
            test_new[dif_y].rename(lambda x: x + '_test', axis=1)], axis=1))

    return [frac_dif_x, frac_dif_y]


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

    for t in ['atolx', 'atoly', 'ltolx', 'ltoly', 'rtolx', 'rtoly']:
        try:
            par[t]
        except KeyError:
            par[t] = None

    rc = pyfunnel.compareAndReport(
        ref.iloc(axis=1)[0],
        ref.iloc(axis=1)[1],
        test.iloc(axis=1)[0],
        test.iloc(axis=1)[1],
        **{k: par[k] for k in ['outputDirectory', 'atolx', 'atoly', 'ltolx', 'ltoly', 'rtolx', 'rtoly']}
    )

    return rc


if __name__ == "__main__":
    pass
