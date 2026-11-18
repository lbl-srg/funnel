#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""The reference lies inside its own tube.

The tube is the reference swept by the rectangle [-atolx, atolx] x [-atoly, atoly],
so at every x the points ref(x) - atoly and ref(x) + atoly are inside it. A test
equal to the reference, or within atoly of it at every sample, must therefore get
an error of exactly zero, whatever atolx is. The cases below each gave a non-zero
error before the bounds were widened to contain these points (containReference
in src/tube.c). The last cases check that the widening hides no real violation.

The data are passed as in-memory arrays rather than read from CSV files: some
failures depend on the last bits of the floating-point values, which the default
pandas CSV parser does not preserve.
"""
import tempfile
import unittest

from test_import import *


def run(x, y, atolx, atoly, xt=None, yt=None, **tol):
    """Compare (xt, yt) with the reference (x, y); return the errors (x, error)."""
    xt = x if xt is None else xt
    yt = y if yt is None else yt
    with tempfile.TemporaryDirectory() as out:
        rc = pyfunnel.compareAndReport(
            xReference=x, yReference=y, xTest=xt, yTest=yt,
            outputDirectory=out, atolx=atolx, atoly=atoly, **tol)
        assert rc == 0, 'compareAndReport returned {}'.format(rc)
        errors = pd.read_csv(os.path.join(out, 'errors.csv'))
    return errors.iloc[:, 0].to_numpy(), errors.iloc[:, 1].abs().to_numpy()


def max_error(*args, **kwargs):
    return run(*args, **kwargs)[1].max()


class TestIdentical(unittest.TestCase):
    """A reference compared with itself."""

    def test_pulse_train_issue_108(self):
        """0/1 pulses with rounding noise on a uniform grid with rounding in x (#108)."""
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
        for atolx in (0.5 * step, 0.999 * step, step, 1.5 * step, 3 * step, 10 * step):
            self.assertEqual(max_error(x, y, atolx, 1e-3), 0, 'atolx = {}'.format(atolx))

    def test_repeated_x_at_event_issue_107(self):
        """x repeated at events, with an output grid point between them (#107)."""
        x = np.array([
            0.078600000000000003, 0.078800000000000009, 0.07899999890361506,
            0.07899999890361506, 0.079000000000000001, 0.079000001096384942,
            0.079000001096384942, 0.079200000000000007, 0.079399999999999998])
        y = np.array([
            6.8160629429994382, 6.5673267313970438, 6.2926738562525131,
            6.2926738562525228, 6.2926738562528941, 6.2926738562525211,
            6.2926738562525211, 6.5673267313970678, 6.8160629429994453])
        self.assertEqual(max_error(x, y, 0.0002, 0.061), 0)

    def test_tiny_magnitude(self):
        """A signal of magnitude 1e-9: the tube is 1e-10 high (the minimum), the size
        of equ()'s absolute floor, so getLower/getUpper merge distinct points."""
        n, step = 2000, 1e-6
        x = step * np.arange(n)
        for amplitude in (2e-9, 5e-10, 2e-10):
            y = amplitude * np.sin(2 * np.pi * x / (n * step / 3))
            for atolx in (step, 3 * step):
                self.assertEqual(max_error(x, y, atolx, 1e-3 * np.ptp(y)), 0,
                                 'amplitude = {}, atolx = {}'.format(amplitude, atolx))

    def test_event_cluster(self):
        """Two events 1e-13 apart (closer than equ() resolves at x = 1) with values
        near zero, then a jump inside the x tolerance."""
        for gap in (2.4e-14, 1e-13, 1e-12):
            x = np.array([1, 1, 1 + gap, 1 + gap, 1 + 1e-4])
            y = np.array([9e-9, 0, 2e-10, 0, 200])
            self.assertEqual(max_error(x, y, 1.1e-4, 2.77), 0, 'gap = {}'.format(gap))

    def test_relative_tolerance(self):
        """Tube height varying along x (rtoly) on the pulse train of #108."""
        x = 0.04288 + 2e-5 * np.arange(40) + 1e-18 * np.arange(40) ** 2
        y = np.where((np.arange(40) // 5) % 2 == 0, 1 - 1e-13, 1e-13)
        self.assertEqual(max_error(x, y, 2e-5, 1e-3, rtoly=1e-2), 0)


class TestInsideTube(unittest.TestCase):
    """A test within atoly of the reference at every sample is inside the tube."""

    def test_offset_below_atoly(self):
        n, step = 2000, 1e-6
        x = step * np.arange(n)
        y = 5e-10 * np.sin(2 * np.pi * x / (n * step / 3))
        atoly = 1e-3 * np.ptp(y)
        rng = np.random.default_rng(0)
        for frac in (0.5, 0.99):
            yt = y + frac * 1e-10 * rng.uniform(-1, 1, n)  # 1e-10 is the minimum tube height
            self.assertEqual(max_error(x, y, step, atoly, yt=yt), 0, 'frac = {}'.format(frac))


class TestViolations(unittest.TestCase):
    """Widening the bounds to ref(x) +/- atoly must not hide a test outside the tube."""

    def test_offset_above_atoly(self):
        """A constant offset of 3 atoly: error 2 atoly where the reference is flat."""
        x = np.linspace(0, 1, 201)
        y = np.where(x < 0.5, 0.0, 1.0)
        atolx, atoly = 0.01, 0.05
        for sign in (1, -1):
            xe, err = run(x, y, atolx, atoly, yt=y + sign * 3 * atoly)
            flat = np.abs(xe - 0.5) > 2 * atolx
            np.testing.assert_allclose(err[flat], 2 * atoly, rtol=1e-9)

    def test_spike_outside_x_window(self):
        """A test that jumps a full x-tolerance window before the reference does."""
        x = np.linspace(0, 1, 201)
        y = np.where(x < 0.5, 0.0, 1.0)
        atolx, atoly = 0.01, 0.05
        yt = np.where(x < 0.5 - 3 * atolx, 0.0, 1.0)
        xe, err = run(x, y, atolx, atoly, yt=yt)
        early = (xe > 0.5 - 3 * atolx + 1e-9) & (xe < 0.5 - atolx - 1e-9)
        self.assertTrue(early.any())
        np.testing.assert_allclose(err[early], 1 - atoly, rtol=1e-9)

    def test_violation_between_reference_points(self):
        """Test samples between coarse reference points, off by 2 atoly."""
        x = np.linspace(0, 1, 11)
        y = x ** 2
        xt = np.linspace(0, 1, 101)
        atoly = 1e-3
        atolx = 1e-6
        _, err = run(x, y, atolx, atoly, xt=xt, yt=np.interp(xt, x, y) + 2 * atoly)
        # The x tolerance raises the upper bound by at most slope * atolx = 2 atolx.
        self.assertTrue(np.all(err[1:-1] <= atoly + 1e-12))
        self.assertTrue(np.all(err[1:-1] >= atoly - 2 * atolx - 1e-12))


if __name__ == '__main__':
    unittest.main()
