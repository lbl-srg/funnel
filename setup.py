#!/usr/bin/env python
# -*- coding: utf-8 -*-

import codecs
import io
import os
import re
from setuptools import setup

MAIN_PACKAGE = 'pyfunnel'
PACKAGE_PATH =  os.path.abspath(os.path.join(os.path.dirname(__file__), MAIN_PACKAGE))

# Version.
version_path = os.path.join(PACKAGE_PATH, 'VERSION')
with open(version_path) as f:
    VERSION = f.read().strip()

# Readme.
readme_path = os.path.abspath(os.path.join(os.path.dirname(__file__), 'README.md'))
with open(readme_path) as f:
    README = f.read()

setup(
    name=MAIN_PACKAGE,
    version=VERSION,
    author='A. Gautier',
    author_email='agautier@lbl.gov',
    url='https://github.com/lbl-srg/funnel',
    description=('Comparison tool for two (x, y) data sets '
        'given tolerances in x and y directions'),
    long_description=README,
    long_description_content_type='text/markdown',
    license="3-clause BSD",
    python_requires='>=3.8',
    install_requires=['six>=1.11'],
    packages=[MAIN_PACKAGE],
    include_package_data=True,
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: C',
        'Intended Audience :: End Users/Desktop',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Utilities',
    ],
)
