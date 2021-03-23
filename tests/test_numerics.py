#!/usr/bin/env python
# -*- coding: utf-8 -*-
from test_import import *

if __name__ == "__main__":
    test_name = sys.argv[1]
    test_dir = sys.argv[2]
    tmp_dir = sys.argv[3]

    rc = run_pyfunnel(test_dir)

    assert rc == 0, "Binary status code for {}: {}.".format(test_dir, rc)

    dif_err = dif_test(test_dir)
    test_log(test_name, test_dir, tmp_dir, dif_err)

    assert max(dif_err) == 0,\
        "Fraction of points that differ between current and reference error values ({}): {} on x, {} on y.".format(
        test_dir, dif_err[0], dif_err[1])
