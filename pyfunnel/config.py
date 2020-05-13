#!/usr/bin/env python
# -*- coding: utf-8 -*-

#######################################################
# Configuration file for pyfunnel
#######################################################

import re
import webbrowser

# Configuration variables - actual values
BROWSER = None


# Configuration variables - default values
DEFAULTS = dict(
    BROWSER=None
)


def save_config(**kwargs):
    '''Save new configuration variables persistently'''
    for k in kwargs.keys():
        with open(__file__, 'r') as f:
            content = f.read()
        if k == 'BROWSER':
            webbrowser.get(kwargs[k])  # Throw exception in case of missing browser.
            target = r'\1{}\n'.format(kwargs[k])
            if kwargs[k] is not None:
                target = r'\1"{}"\n'.format(kwargs[k])  # Add quotes for strings.
            content = re.sub('({} = )(.*)\n'.format(k), target, content)
        with open(__file__, 'w') as f:
            f.write(content)


def reinit():
    '''Reinitialize configuration variables to default values'''
    save_config(**DEFAULTS)


