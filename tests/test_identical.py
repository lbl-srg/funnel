#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Comparing a series with itself must give an error of exactly zero.

The data are passed as in-memory arrays rather than read from CSV files: the
failures depend on the last bits of the floating-point values, which the
default pandas CSV parser does not preserve.
"""
import tempfile
import unittest

from test_import import *


class TestIdentical(unittest.TestCase):

    def max_error(self, x, y, atolx, atoly):
        """Compare (x, y) with itself and return the largest error."""
        with tempfile.TemporaryDirectory() as out:
            rc = pyfunnel.compareAndReport(
                xReference=x, yReference=y, xTest=x, yTest=y,
                outputDirectory=out, atolx=atolx, atoly=atoly)
            self.assertEqual(rc, 0)
            errors = pd.read_csv(os.path.join(out, 'errors.csv'))
        return errors.iloc[:, 1].abs().max()

    def test_repeated_x_at_event(self):
        x = np.array([
            0.078600000000000003,
            0.078800000000000009,
            0.07899999890361506,
            0.07899999890361506,  # event: repeated x
            0.079000000000000001,  # output grid point
            0.079000001096384942,
            0.079000001096384942,  # event: repeated x
            0.079200000000000007,
            0.079399999999999998])
        y = np.array([
            6.8160629429994382,
            6.5673267313970438,
            6.2926738562525131,
            6.2926738562525228,
            6.2926738562528941,
            6.2926738562525211,
            6.2926738562525211,
            6.5673267313970678,
            6.8160629429994453])
        self.assertEqual(self.max_error(x, y, atolx=0.0002, atoly=0.061), 0)
        # Same data without the repeats.
        keep = np.concatenate(([True], np.diff(x) > 0))
        self.assertEqual(self.max_error(x[keep], y[keep], atolx=0.0002, atoly=0.061), 0)

    def test_pulse_train_with_rounding_noise(self):
        x = [
            0.042879999999999995, 0.042899999999999994, 0.04291999999999999, 0.04294,
            0.04296, 0.04298, 0.043, 0.043019999999999996, 0.043039999999999995,
            0.043059999999999994, 0.04307999999999999, 0.0431, 0.04312, 0.04314,
            0.04316, 0.043179999999999996, 0.043199999999999995, 0.043219999999999995,
            0.043239999999999994, 0.04325999999999999, 0.04328, 0.0433, 0.04332,
            0.04334, 0.043359999999999996, 0.043379999999999995, 0.043399999999999994,
            0.04341999999999999, 0.04344, 0.04346, 0.04348, 0.0435, 0.043519999999999996,
            0.043539999999999995, 0.043559999999999995, 0.043579999999999994,
            0.04359999999999999, 0.04362, 0.04364, 0.04366]
        y = [
            0.9999999999998366, 0.9999999999998863, 0.9999999999999005,
            0.9999999999999005, 0.999999999999936, 0.0, 3.552713678800501e-14,
            7.105427357601002e-14, 6.394884621840902e-14, 0.0, 0.9999999999999574,
            0.999999999999936, 0.999999999999929, 0.9999999999998934,
            0.9999999999998863, 0.9999999999998721, 0.9999999999998721,
            0.9999999999999503, 0.9999999999999929, 0.0, 0.0, 7.105427357601002e-15,
            2.1316282072803006e-14, 0.0, 0.9999999999999218, 0.9999999999998863,
            0.999999999999929, 0.9999999999999858, 0.9999999999999574,
            0.9999999999999574, 2.1316282072803006e-14, 1.2789769243681803e-13,
            1.9184653865522705e-13, 1.9895196601282805e-13, 1.2789769243681803e-13,
            4.973799150320701e-14, 0.9999999999999645, 0.9999999999998721,
            0.9999999999997655, 0.9999999999997371]
        step = 2e-5
        for atolx in (0.5 * step, 0.999 * step, step, 1.5 * step, 3 * step):
            self.assertEqual(self.max_error(x, y, atolx=atolx, atoly=1e-3), 0,
                             'atolx = {}'.format(atolx))

    def test_noisy_ramp_with_fine_grid(self):
        """Interpolating the tube on a fine grid (step below 1e-5) must not flatten it."""
        n = 45
        step = 1.0868835117526014e-06
        x = 0.2493819342 + step * np.arange(n)
        y = np.abs(np.linspace(-1, 1, n)) + 1
        for atolx in (0.5 * step, step, 1.5 * step):
            self.assertEqual(self.max_error(x, y, atolx=atolx, atoly=6e-4), 0,
                             'atolx = {}'.format(atolx))


if __name__ == '__main__':
    unittest.main()
