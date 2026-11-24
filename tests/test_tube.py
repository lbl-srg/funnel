#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""Tests of the tube: what is inside it and what is not.

The tube is the reference swept by the rectangle [-atolx, atolx] x [-atoly, atoly].
It is a closed set: its edge belongs to it. A test point (x, y) is therefore
inside the tube when the reference takes a value within atoly of y somewhere
within atolx of x. In particular

- a test identical to the reference has an error of exactly zero,
- so has a test within atoly of the reference at every sample, and a test equal
  to the reference shifted along x by atolx or less.

Most cases below gave a non-zero error when the construction of the tube
compared floating-point values with a tolerance (issues #107 and #108, pull
requests #109 and #110). The last class compares the errors with the ones
computed from the definition above, on random data.

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
        # Default parser: reads back the 16 significant digits written by funnel.
        errors = pd.read_csv(os.path.join(out, 'errors.csv'), float_precision='round_trip')
    return errors.iloc[:, 0].to_numpy(), errors.iloc[:, 1].abs().to_numpy()


def max_error(*args, **kwargs):
    return run(*args, **kwargs)[1].max()


def exact_bounds(x, y, xt, atolx=0.0, atoly=0.0, ltolx=0.0, ltoly=0.0, scale_x=1.0):
    """Lower and upper bounds of the tube at xt, from its definition, without building its outline.

    The upper bound at x is the largest of
    - the tops y_i + ty_i of the rectangles of the reference points with |x_i - x| <= tx_i,
    - the reference shifted by (-tx, +ty) and the reference shifted by (+tx, +ty), at x.
    The lower bound is symmetric. With ltolx or ltoly, the size (tx_i, ty_i) of
    the rectangle varies from one reference point to the next, and linearly in
    between. `scale_x` scales tx: it is used to find the samples that sit on a
    vertical edge of the tube.
    """
    x, y, xt = (np.asarray(v, dtype=float) for v in (x, y, xt))
    tx = np.maximum(atolx, ltolx * np.abs(x)) * scale_x
    ty = np.maximum(atoly, ltoly * np.abs(y))
    assert tx.min() > 1e-10 and ty.min() > 1e-10, 'funnel replaces a tube size below 1e-10'
    inside = (x[None, :] - tx[None, :] <= xt[:, None]) & (xt[:, None] <= x[None, :] + tx[None, :])
    upper = np.where(inside, (y + ty)[None, :], -np.inf).max(axis=1)
    lower = np.where(inside, (y - ty)[None, :], np.inf).min(axis=1)
    for xs in (x - tx, x + tx):
        upper = np.maximum(upper, np.interp(xt, xs, y + ty, left=-np.inf, right=-np.inf))
        lower = np.minimum(lower, np.interp(xt, xs, y - ty, left=np.inf, right=np.inf))
    return lower, upper


def exact_errors(x, y, xt, yt, **tol):
    """Errors computed from the definition of the tube, see exact_bounds."""
    lower, upper = exact_bounds(x, y, xt, **tol)
    yt = np.asarray(yt, dtype=float)
    return np.maximum(np.maximum(lower - yt, yt - upper), 0)


def bounds(x, y, **tol):
    """Lower and upper tube curves written by funnel, as arrays of (x, y)."""
    with tempfile.TemporaryDirectory() as out:
        rc = pyfunnel.compareAndReport(xReference=x, yReference=y, xTest=x, yTest=y,
                                       outputDirectory=out, **tol)
        assert rc == 0, 'compareAndReport returned {}'.format(rc)
        return tuple(pd.read_csv(os.path.join(out, f), float_precision='round_trip').to_numpy()
                     for f in ('lowerBound.csv', 'upperBound.csv'))


# 0/1 pulses with rounding noise, on a uniform grid with rounding in x (issue #108).
X_108 = np.array([
    0.042879999999999995, 0.042899999999999994, 0.04291999999999999, 0.04294,
    0.04296, 0.04298, 0.043, 0.043019999999999996, 0.043039999999999995,
    0.043059999999999994, 0.04307999999999999, 0.0431, 0.04312, 0.04314,
    0.04316, 0.043179999999999996, 0.043199999999999995, 0.043219999999999995,
    0.043239999999999994, 0.04325999999999999, 0.04328, 0.0433, 0.04332,
    0.04334, 0.043359999999999996, 0.043379999999999995, 0.043399999999999994,
    0.04341999999999999, 0.04344, 0.04346, 0.04348, 0.0435, 0.043519999999999996,
    0.043539999999999995, 0.043559999999999995, 0.043579999999999994,
    0.04359999999999999, 0.04362, 0.04364, 0.04366])
Y_108 = np.array([
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
    0.9999999999997655, 0.9999999999997371])
STEP_108 = 2e-5

# x repeated at events, with an output grid point between them (issue #107).
X_107 = np.array([
    0.078600000000000003, 0.078800000000000009,
    0.07899999890361506, 0.07899999890361506,    # event: repeated x
    0.079000000000000001,                        # output grid point
    0.079000001096384942, 0.079000001096384942,  # event: repeated x
    0.079200000000000007, 0.079399999999999998])
Y_107 = np.array([
    6.8160629429994382, 6.5673267313970438, 6.2926738562525131,
    6.2926738562525228, 6.2926738562528941, 6.2926738562525211,
    6.2926738562525211, 6.5673267313970678, 6.8160629429994453])
ATOLX_107, ATOLY_107 = 0.0002, 0.061


class TestIdentical(unittest.TestCase):
    """A reference compared with itself: the error is exactly zero."""

    def test_pulse_train_issue_108(self):
        for atolx in (0.5, 0.999, 1, 1.5, 3, 10):
            self.assertEqual(max_error(X_108, Y_108, atolx * STEP_108, 1e-3), 0,
                             'atolx = {} * step'.format(atolx))

    def test_repeated_x_at_event_issue_107(self):
        self.assertEqual(max_error(X_107, Y_107, ATOLX_107, ATOLY_107), 0)
        # Same data without the repeats.
        keep = np.concatenate(([True], np.diff(X_107) > 0))
        self.assertEqual(max_error(X_107[keep], Y_107[keep], ATOLX_107, ATOLY_107), 0)

    def test_fine_grid(self):
        """A grid with a step below 1e-5: the tube must be interpolated on it, not held."""
        n = 45
        step = 1.0868835117526014e-06
        x = 0.2493819342 + step * np.arange(n)
        y = np.abs(np.linspace(-1, 1, n)) + 1
        for atolx in (0.5, 1, 1.5):
            self.assertEqual(max_error(x, y, atolx * step, 6e-4), 0,
                             'atolx = {} * step'.format(atolx))

    def test_slow_ramp_on_large_values(self):
        """Steps below 1e-10 relative to y are not flat: they add up to 500 times atoly."""
        x = np.arange(1000.0)
        for slope in (-50, 50):
            self.assertEqual(max_error(x, 1e12 + slope * x, 10, 100), 0, 'slope = {}'.format(slope))

    def test_slow_tail_on_large_values(self):
        """A steep ramp followed by a slow one: the corner at the end is kept."""
        x = np.arange(110.0)
        y = 1e11 + np.where(x < 100, 1000 * x, 1000 * 99 + 5 * (x - 99))
        self.assertEqual(max_error(x, y, 10, 5), 0)

    def test_tiny_magnitude(self):
        """A signal of magnitude 1e-9: the tube has its minimum height, 1e-10."""
        n, step = 2000, 1e-6
        x = step * np.arange(n)
        for amplitude in (2e-9, 5e-10, 2e-10):
            y = amplitude * np.sin(2 * np.pi * x / (n * step / 3))
            for atolx in (step, 3 * step):
                self.assertEqual(max_error(x, y, atolx, 1e-3 * np.ptp(y)), 0,
                                 'amplitude = {}, atolx = {}'.format(amplitude, atolx))

    def test_event_cluster(self):
        """Two events 1e-13 apart with values near zero, then a jump within atolx."""
        for gap in (2.4e-14, 1e-13, 1e-12):
            x = np.array([1, 1, 1 + gap, 1 + gap, 1 + 1e-4])
            y = np.array([9e-9, 0, 2e-10, 0, 200])
            self.assertEqual(max_error(x, y, 1.1e-4, 2.77), 0, 'gap = {}'.format(gap))

    def test_relative_tolerance(self):
        """Pulse train of #108 with a tube height set by rtoly."""
        x = 0.04288 + 2e-5 * np.arange(40) + 1e-18 * np.arange(40) ** 2
        y = np.where((np.arange(40) // 5) % 2 == 0, 1 - 1e-13, 1e-13)
        self.assertEqual(max_error(x, y, 2e-5, 1e-3, rtoly=1e-2), 0)

    def test_local_tolerance(self):
        """Tube size varying from one reference point to the next (ltolx, ltoly)."""
        x = 1 + 0.05 * np.arange(200)
        y = 10 * np.sin(x) + 3  # crosses zero: the tube shrinks to atoly there
        self.assertEqual(max_error(x, y, 1e-3, 1e-3, ltolx=0.02, ltoly=0.02), 0)


class TestInsideTube(unittest.TestCase):
    """A test that differs from the reference by less than the tolerances: zero error."""

    def test_offset_below_atoly(self):
        n, step = 2000, 1e-6
        x = step * np.arange(n)
        y = 5e-10 * np.sin(2 * np.pi * x / (n * step / 3))
        atoly = 1e-3 * np.ptp(y)
        rng = np.random.default_rng(0)
        for frac in (0.5, 0.99):
            yt = y + frac * 1e-10 * rng.uniform(-1, 1, n)  # 1e-10 is the minimum tube height
            self.assertEqual(max_error(x, y, step, atoly, yt=yt), 0, 'frac = {}'.format(frac))

    def test_shift_below_atolx_at_event(self):
        """The reference of #107 shifted along x by less than atolx."""
        for frac in (-0.9, -0.5, -0.25, 0.25, 0.5, 0.9):
            shift = frac * ATOLX_107
            xt = np.unique(np.concatenate([X_107, X_107 + shift]))
            xt = xt[(xt >= X_107[0]) & (xt <= X_107[-1])]
            yt = np.interp(xt - shift, X_107, Y_107)
            self.assertEqual(max_error(X_107, Y_107, ATOLX_107, ATOLY_107, xt=xt, yt=yt), 0,
                             'shift = {} * atolx'.format(frac))


class TestTubeEdge(unittest.TestCase):
    """The edge of the tube belongs to the tube, along x as along y."""

    atolx, atoly = 0.01, 0.01
    x = np.array([0.0, 1.0, 1.0, 2.0])  # step from 0 to 1 at x = 1
    y = np.array([0.0, 0.0, 1.0, 1.0])

    def test_event_at_atolx(self):
        """The test steps at exactly atolx before, or after, the reference does."""
        for xe in (1.0 - self.atolx, 1.0 + self.atolx):
            xt = np.array([0.0, xe, xe, 2.0])
            self.assertEqual(max_error(self.x, self.y, self.atolx, self.atoly, xt=xt, yt=self.y), 0,
                             'event at {}'.format(xe))

    def test_event_at_atolx_and_atoly(self):
        """Same, with the values at exactly atoly from the reference."""
        for xe in (1.0 - self.atolx, 1.0 + self.atolx):
            xt = np.array([0.0, xe, xe, 2.0])
            yt = np.array([0.0, 0.0 - self.atoly, 1.0 + self.atoly, 1.0])
            self.assertEqual(max_error(self.x, self.y, self.atolx, self.atoly, xt=xt, yt=yt), 0,
                             'event at {}'.format(xe))

    def test_event_beyond_atolx(self):
        """The test steps at twice atolx from the reference: the step is outside."""
        for xe, i in ((1.0 - 2 * self.atolx, 2), (1.0 + 2 * self.atolx, 1)):
            xt = np.array([0.0, xe, xe, 2.0])
            _, err = run(self.x, self.y, self.atolx, self.atoly, xt=xt, yt=self.y)
            expected = np.zeros(4)
            expected[i] = 1 - self.atoly
            np.testing.assert_allclose(err, expected, rtol=1e-12, atol=0)

    def test_first_sample_at_atolx_from_event(self):
        """An event at exactly atolx from the first sample: both of its values are allowed there."""
        x = np.array([0.0, 1.0, 1.0, 2.0, 3.0])
        y = np.array([0.0, -0.2, -1.2, 1.1, 1.0])  # jumps from -0.2 to -1.2 at x = 1
        xt = np.array([0.0, 1.5, 3.0])
        for side in (1, -1):  # -1: mirrored, the edge is on the upper curve
            for y0 in (0.01, -0.2, -1.2, -1.2 - 0.01):
                yt = np.array([y0, 1.0, 1.0])
                self.assertEqual(max_error(x, side * y, 1.0, 0.01, xt=xt, yt=side * yt), 0,
                                 'side = {}, y0 = {}'.format(side, y0))
            # Beyond the edge of the tube along y.
            _, err = run(x, side * y, 1.0, 0.01, xt=xt, yt=side * np.array([-1.5, 1.0, 1.0]))
            np.testing.assert_allclose(err, [1.5 - 1.2 - 0.01, 0, 0], rtol=1e-12, atol=0)


class TestViolations(unittest.TestCase):
    """A test outside the tube: the error is its distance to the tube along y."""

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
        """A test that jumps three x tolerances before the reference does."""
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

    def test_slow_ramp_on_large_values(self):
        """An offset of 8 atoly on a slow ramp: atoly and the x tolerance cover 6 atoly of it."""
        x = np.arange(1000.0)
        y = 1e12 - 50 * x
        atolx, atoly = 10.0, 100.0
        for sign in (1, -1):
            _, err = run(x, y, atolx, atoly, yt=y + sign * 8 * atoly)
            # The tube spans y +/- (atoly + 50 * atolx) away from the ends.
            np.testing.assert_allclose(err[20:-20], 8 * atoly - (atoly + 50 * atolx), rtol=1e-6)


class TestLoopRemoval(unittest.TestCase):
    """Where a tube curve doubles back along x, the loop is cut at its crossing point."""

    def test_nearly_coincident_segments(self):
        """Values of order 1e-18 before a step: the crossing segments of the lower curve are
        horizontal and 1 ulp apart, so their intersection point is ill-conditioned."""
        x = np.arange(6.0)
        y = np.array([-3.6040282968004712e-18, 3.1697801862521394e-18, 2.2247990547886374e-18,
                      -1.0, -1.0, -1.0])
        tol = dict(atolx=0.5, atoly=0.061)
        for curve in bounds(x, y, **tol):
            self.assertTrue(np.all(np.diff(curve[:, 0]) >= 0), 'x goes backwards: {}'.format(curve))
        xt = np.linspace(0, 5, 500)  # no sample on a vertical edge (x_i +/- atolx)
        lower, upper = exact_bounds(x, y, xt, **tol)
        for yt in (lower, upper, lower - tol['atoly'] / 2, upper + tol['atoly'] / 2):
            _, err = run(x, y, xt=xt, yt=yt, **tol)
            np.testing.assert_allclose(err, exact_errors(x, y, xt, yt, **tol), rtol=0, atol=1e-12)


class TestAgainstDefinition(unittest.TestCase):
    """Errors on random data equal the ones computed from the definition of the tube.

    Not compared: the samples on a vertical edge of the tube, where the error
    jumps (the choice made there is tested in TestTubeEdge). Differences are
    accepted up to what a few units in the last place of x or y do to the data.
    """

    KINDS = ('random walk', 'pulses with rounding noise', 'plateaus',
             'slow drift on a large value', 'events', 'local tolerances')

    def reference(self, rng, kind):
        n = int(rng.integers(8, 120))
        step = float(rng.choice([1.0, 0.1, 2e-5, 1 / 3, 900.0]))
        x = rng.choice([0.0, 0.04288, 3.15e7]) + step * np.arange(n)
        tol = dict()
        if kind == 'random walk':
            y = np.cumsum(rng.normal(0, 1, n))
        elif kind == 'pulses with rounding noise':
            y = (np.sin(np.arange(n) / rng.uniform(1, 10)) > 0) * 1.0 + rng.normal(0, 1e-13, n)
        elif kind == 'plateaus':
            y = np.round(np.cumsum(rng.normal(0, 1, n))) + rng.normal(0, 1, n) * 10 ** rng.uniform(-22, -15)
        elif kind == 'slow drift on a large value':
            y = 10 ** rng.uniform(9, 12) * (1 + 1e-9 * np.cumsum(rng.normal(0, 1, n)))
        elif kind == 'events':
            y = np.cumsum(rng.normal(0, 1, n))
            k = np.sort(rng.integers(1, n - 1, max(1, n // 8)))
            order = np.argsort(np.concatenate([np.arange(n), k + 0.5]), kind='stable')
            # 40 % of the repeated x hold the same y twice, 60 % a jump.
            yk = np.where(rng.random(len(k)) < 0.4, y[k], rng.normal(0, 3, len(k)))
            x, y = np.concatenate([x, x[k]])[order], np.concatenate([y, yk])[order]
        else:
            x = 1 + 0.1 * np.cumsum(rng.uniform(0.2, 1.8, n))
            y = 10 * np.sin(x / rng.uniform(1, 10)) + rng.choice([0.0, 3.0])
            tol = dict(ltolx=float(rng.choice([1e-3, 1e-2, 0.05])), ltoly=float(rng.choice([1e-3, 1e-2, 0.05])))
            step = 0.1
        tol['atolx'] = float(step * rng.choice([0.3, 0.5, 1.0, 1.5, 3.0]))
        tol['atoly'] = float(max(np.ptp(y), 1.0) * rng.choice([1e-3, 1e-2, 0.1]))
        return x, y, tol

    def test_random_data(self):
        rng = np.random.default_rng(2026)
        n_compared = 0
        for case in range(240):
            kind = self.KINDS[case % len(self.KINDS)]
            x, y, tol = self.reference(rng, kind)
            atolx, atoly = tol['atolx'], tol['atoly']
            msg = 'case {} ({}), tolerances {}'.format(case, kind, tol)

            self.assertEqual(max_error(x, y, **tol), 0, 'Reference against itself, ' + msg)

            # Reference shifted along x by up to 1.2 atolx, with noise of 0.7 atoly along y:
            # inside the tube at some samples, outside at others.
            dx = np.diff(x)
            slope = np.abs(np.diff(y)[dx > 0] / dx[dx > 0]).max()
            resolution = 16 * (np.spacing(np.abs(y).max()) + slope * np.spacing(np.abs(x).max()))
            for xt in (x, np.sort(np.concatenate([x[[0, -1]], rng.uniform(x[0], x[-1], len(x))]))):
                shift = rng.uniform(-1.2, 1.2) * atolx
                yt = np.interp(np.clip(xt + shift, x[0], x[-1]), x, y) + rng.normal(0, 0.7 * atoly, len(xt))
                _, err = run(x, y, xt=xt, yt=yt, **tol)
                expected = exact_errors(x, y, xt, yt, **tol)
                # On a vertical edge of the tube: the error changes with the width of the tube.
                # Away from an edge, it changes by the slope times the change in width at most.
                eps = max(1e-9, 4 * np.spacing(np.abs(x).max()) / atolx)
                narrower = exact_errors(x, y, xt, yt, scale_x=1 - eps, **tol)
                wider = exact_errors(x, y, xt, yt, scale_x=1 + eps, **tol)
                threshold = resolution + 1e-6 * atoly
                tx_max = max(atolx, tol.get('ltolx', 0) * np.abs(x).max())
                on_edge = np.abs(narrower - wider) > threshold + 4 * eps * slope * tx_max
                bad = (np.abs(err - expected) > threshold) & ~on_edge
                n_compared += int((~on_edge).sum())
                if bad.any():
                    i = np.where(bad)[0][0]
                    self.fail('Error {!r} instead of {!r} at x = {!r}, {}'.format(
                        err[i], expected[i], xt[i], msg))
        self.assertGreater(n_compared, 20000)


if __name__ == '__main__':
    unittest.main()
