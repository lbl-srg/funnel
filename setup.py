#!/usr/bin/env python
# coding: utf-8

import platform
import os
from distutils.core import setup
from funnel import __version__


# Detect path of binary dependencies.
os_name = platform.system()
lib_data = 'funnel/lib'
if os_name == 'Windows':
    lib_data = '{}/win64/*.dll'.format(lib_data)
elif os_name == 'Linux':
    lib_data = '{}/linux64/*.so'.format(lib_data)
elif os_name == 'Darwin':
    lib_data = '{}/darwin64/*.dylib'.format(lib_data)
else:
    raise RuntimeError('Could not detect standard (system, architecture).')


setup(
    name='funnel',
    version=__version__,
    author='A. Gautier, J. Hu',
    url='https://github.com/lbl-srg/funnel',
    description=('Comparison tool for two (x, y) data sets '
        'given tolerances in x and y directions.'),
    license="3-clause BSD",
    python_requires='>=2.7, >=3.6',
    install_requires=['six>=1.11'],
    packages=['funnel'],
    package_data={
        '': ['templates/*', lib_data],
    },
    include_package_data=True,
    classifiers=[
        'Development Status :: 5 - Production/Stable',
        'Environment :: Console',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: C',
        'Intended Audience :: End Users/Desktop',
        'Intended Audience :: Science/Research',
        'Topic :: Scientific/Engineering',
        'Topic :: Utilities',
    ],
)