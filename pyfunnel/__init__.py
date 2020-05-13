#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
    Python binding for funnel library.
"""

from __future__ import absolute_import

import os

from .core import compareAndReport, MyHTTPServer, CORSRequestHandler, plot_funnel

# Version.
version_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'VERSION'))
with open(version_path) as f:
    __version__ = f.read().strip()

