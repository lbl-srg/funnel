#!/usr/bin/env python
# -*- coding: utf-8 -*-

import codecs
import io
import platform
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
with io.open(readme_path, encoding='utf-8') as f:  # io.open for Python 2 support with encoding
    README = f.read()

# Library path.
os_name = platform.system()
lib_data = 'lib'  # Relative path in MAIN_PACKAGE.
if os_name == 'Windows':
    lib_data = '{}/win64/*.dll'.format(lib_data)
elif os_name == 'Linux':
    lib_data = '{}/linux64/*.so'.format(lib_data)
elif os_name == 'Darwin':
    lib_data = '{}/darwin64/*.dylib'.format(lib_data)
else:
    raise RuntimeError('Could not detect standard (system, architecture).')

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
    python_requires='>=2.7, !=3.0.*, !=3.1.*, !=3.2.*, !=3.3.*, !=3.4.*, !=3.5.*',
    install_requires=['six>=1.11'],
    packages=[MAIN_PACKAGE],
    package_data={
        MAIN_PACKAGE: ['VERSION', lib_data],
    },
    include_package_data=True,
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: C',
        'Intended Audience :: End Users/Desktop',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Utilities',
    ],
)
