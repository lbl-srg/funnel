#!/usr/bin/env python
# -*- coding: utf-8 -*-
import unittest
from unittest import mock

from test_import import *

import pyfunnel.core as core


class TestLibPath(unittest.TestCase):

    def lib_dir(self, system, python_platform, machine='AMD64'):
        """Return the library directory selected for the given platform."""
        with mock.patch.object(core.platform, 'system', return_value=system), \
                mock.patch.object(core.platform, 'machine', return_value=machine), \
                mock.patch.object(core.sysconfig, 'get_platform', return_value=python_platform):
            return os.path.basename(os.path.dirname(core._get_lib_path('funnel')))

    def test_windows(self):
        """The directory depends on the architecture of Python, not of the host."""
        self.assertEqual(self.lib_dir('Windows', 'win-amd64', machine='ARM64'), 'win64')
        self.assertEqual(self.lib_dir('Windows', 'win-arm64', machine='ARM64'), 'winarm64')
        self.assertEqual(self.lib_dir('Windows', 'win32', machine='AMD64'), 'win32')
        with self.assertRaisesRegex(RuntimeError, 'win-arm32'):
            self.lib_dir('Windows', 'win-arm32')

    def test_linux(self):
        self.assertEqual(self.lib_dir('Linux', 'linux-x86_64', machine='x86_64'), 'linux64')
        self.assertEqual(self.lib_dir('Linux', 'linux-aarch64', machine='aarch64'), 'linuxarm64')
        with mock.patch.object(core.sys, 'maxsize', 2**31 - 1):  # 32-bit Python
            self.assertEqual(self.lib_dir('Linux', 'linux-x86_64', machine='x86_64'), 'linux32')
        with self.assertRaisesRegex(RuntimeError, 'ppc64le'):
            self.lib_dir('Linux', 'linux-ppc64le', machine='ppc64le')

    def test_missing_lib(self):
        """A missing library raises an error listing the available libraries."""
        lib_root = os.path.join(os.path.dirname(core.__file__), 'lib')
        lib_path = os.path.join(lib_root, 'no-such-platform', 'funnel.dll')
        with self.assertRaises(RuntimeError) as cm:
            core._load_lib(lib_path)
        msg = str(cm.exception)
        self.assertIn(lib_path, msg)
        for d in os.listdir(lib_root):
            self.assertIn(d, msg)


if __name__ == '__main__':
    unittest.main(verbosity=2)
