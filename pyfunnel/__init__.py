#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Python binding for funnel library.
"""

# Main public API functions that users should be able to import directly from pyfunnel
from .core import CORSRequestHandler, MyHTTPServer, compareAndReport, plot_funnel

__all__ = ['CORSRequestHandler', 'MyHTTPServer', 'compareAndReport', 'plot_funnel']
__version__ = '2.0.0'  # DO NOT CHANGE: this is automatically updated with 'cz bump'
