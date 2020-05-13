#!/usr/bin/env python
# -*- coding: utf-8 -*-
import time
import threading
from test_import import *

if __name__ == "__main__":
    test_dir = sys.argv[1]
    pyfunnel.plot_funnel(test_dir)
    time.sleep(1)  # to catch thread exception
    sys.stderr.flush()  # to output thread exception

    sys.exit()
