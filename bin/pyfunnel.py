#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
#######################################################
# Python binding for funnel library: compareAndReport
# Function that plots results: plot_funnel
#######################################################
from __future__ import absolute_import, division, print_function, unicode_literals
# Python standard library imports.
from ctypes import cdll, POINTER, c_double, c_int, c_char_p
import io
import numbers
import os
import platform
import re
import subprocess
import sys
import textwrap
import threading
import time
import warnings
import webbrowser
try:
    from http.server import HTTPServer, SimpleHTTPRequestHandler # Python 3
except ImportError:
    from SimpleHTTPServer import BaseHTTPServer
    HTTPServer = BaseHTTPServer.HTTPServer
    from SimpleHTTPServer import SimpleHTTPRequestHandler # Python 2
# Third-party module or package imports.
import six
# Code repository sub-package imports.


def _get_lib_path(project_name):
    """Infers the library absolute path.

    Args:
        project_name (str): project name

    Returns:
        str: guessed library path e.g. ~/project_name/lib/darwin64/lib{project_name}.so
    """
    lib_path = os.path.join(os.path.dirname(__file__), os.path.pardir, 'lib')
    os_name = platform.system()
    os_machine = platform.machine()
    if os_name == 'Windows':
        if os_machine.endswith('64'):
            lib_path = os.path.join(lib_path, 'win64', '{}.dll'.format(project_name))
        else:
            lib_path = os.path.join(lib_path, 'win32', '{}.dll'.format(project_name))
    elif os_name == 'Linux':
        if os_machine.endswith('64'):
            lib_path = os.path.join(lib_path, 'linux64', 'lib{}.so'.format(project_name))
        else:
            lib_path = os.path.join(lib_path, 'linux32', 'lib{}.so'.format(project_name))
    elif os_name == 'Darwin':
        lib_path = os.path.join(lib_path, 'darwin64', 'lib{}.dylib'.format(project_name))
    else:
        raise RuntimeError('Could not detect standard (system, architecture).')

    return os.path.abspath(lib_path)


def compareAndReport(
    xReference,
    yReference,
    xTest,
    yTest,
    outputDirectory=None,
    atolx=None,
    atoly=None,
    rtolx=None,
    rtoly=None
):
    """Runs funnel binary with list-like objects as x, y reference and test values.

    Outputs `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`,
    `test.csv` into the output directory (`./results` by default).

    Args:
        xReference (list-like of floats): x reference values
        yReference (list-like of floats): y reference values
        xTest (list-like of floats): x test values
        yTest (list-like of floats): y test values
        outputDirectory (str): path of directory to store output files
        atolx (float): absolute tolerance along x axis
        atoly (float): absolute tolerance along y axis
        rtolx (float): relative tolerance along x axis
        rtoly (float): relative tolerance along y axis

    Returns:
        None

    Note: At least one absolute or relative tolerance parameter must be provided for each axis.
    Relative tolerance is relative to the range of x or y values.

    Full documentation at https://github.com/lbl-srg/funnel.
    """

    # Check arguments.
    ## Logic
    assert (atolx is not None) or (rtolx is not None),\
        "At least one of the two possible tolerance parameters (atol or rtol) must be defined for x values."
    assert (atoly is not None) or (rtoly is not None),\
        "At least one of the two possible tolerance parameters (atol or rtol) must be defined for y values."
    ## Type
    if outputDirectory is None:
        print("Output directory not specified: results are stored in subdirectory `results` by default.")
        outputDirectory = "results"
    assert isinstance(outputDirectory, six.string_types),\
        "Path of output directory is not a string type."
    ## Value
    assert len(xReference) == len(yReference),\
        "xReference and yReference must have the same length."
    assert len(xTest) == len(yTest),\
        "xTest and yTest must have the same length."

    # Convert arrays into lists (to support np.array and pd.Series).
    try:
        xReference = list(xReference)
        yReference = list(yReference)
        xTest = list(xTest)
        yTest = list(yTest)
    except Exception as e:
        raise TypeError("Input data could not be converted into lists: {}".format(e))
    # Test numeric type.
    all_data = xReference + yReference + xTest + yTest
    num_check = [isinstance(x, numbers.Real) for x in all_data]
    if not min(num_check):
        idx = filter(lambda i: not num_check[i], range(len(num_check)))
        raise TypeError("The following input values are not numeric: {}".format(
            [all_data[i] for i in idx]
        ))

    # Convert None tolerance to 0.
    tol = dict()
    args = locals()
    for k in ('atolx', 'atoly', 'rtolx', 'rtoly'):
        if args[k] is None:
            tol[k] = 0.0
        else:
            try:
                tol[k] = float(args[k])
            except:
                raise TypeError("Tolerance {} could not be converted to float.".format(k))
            if tol[k] < 0:
                raise ValueError("Tolerance {} must be positive.".format(k))

    # Encode string arguments (in Python 3 c_char_p takes bytes object).
    outputDirectory = outputDirectory.encode('utf-8')

    # Load library.
    try:
        lib_path = _get_lib_path('funnel')
        lib = cdll.LoadLibrary(lib_path)
    except Exception as e:
        raise RuntimeError("Could not load funnel library with this path: {}. {}".format(lib_path, e))

    # Map arguments.
    lib.compareAndReport.argtypes = [
        POINTER(c_double),
        POINTER(c_double),
        c_int,
        POINTER(c_double),
        POINTER(c_double),
        c_int,
        c_char_p,
        c_double,
        c_double,
        c_double,
        c_double]
    lib.compareAndReport.restype = c_int

    # Run
    try:
        retVal = lib.compareAndReport(
            (c_double * len(xReference))(*xReference),
            (c_double * len(yReference))(*yReference),
            len(xReference),
            (c_double * len(xTest))(*xTest),
            (c_double * len(yTest))(*yTest),
            len(xTest),
            outputDirectory,
            tol['atolx'],
            tol['atoly'],
            tol['rtolx'],
            tol['rtoly']
        )
    except Exception as e:
        raise RuntimeError("Library call raises exception: {}.".format(e))
    if retVal != 0:
        warnings.warn("funnel binary status code is: {}.".format(retVal), RuntimeWarning)

    return retVal


class MyHTTPServer(HTTPServer):
    """Adds custom server_launch, server_close and browse methods."""

    def __init__(self, *args, **kwargs):
        """kwargs:

            str_html (str): HTML content to serve if URL ends with url_html
            url_html (str): pattern used to serve str_html if URL ends with it
            browse_dir (str): path of directory where to launch the server
        """
        str_html = kwargs.pop('str_html', None)
        url_html = kwargs.pop('url_html', None)
        browse_dir = kwargs.pop('browse_dir', os.getcwd())
        HTTPServer.__init__(self, *args)
        self._STR_HTML = re.sub('\$SERVER_PORT', str(self.server_port), str_html)
        self._URL_HTML = url_html
        self._BROWSE_DIR = browse_dir
        self.logger = io.BytesIO()

    def server_launch(self):
        self.thread = threading.Thread(target=self.serve_forever)
        self.thread.daemon = True  # daemonic thread objects are terminated as soon as the main thread exits
        self.thread.start()

    def server_close(self):
        # Invoke to close logger.
        threadd = threading.Thread(target=self.shutdown)  # makes execution stall on Windows if main thread
        threadd.daemon = True
        threadd.start()
        try:
            self.logger.close()
        except Exception as e:
            print('Could not close logger: {}'.format(e))

    def browse(self, *args, **kwargs):
        # TODOC
        browser = kwargs.pop('browser', None)
        timeout = kwargs.pop('timeout', 5)
        # Move to directory with *.csv before launching local server.
        cur_dir = os.getcwd()
        os.chdir(self._BROWSE_DIR)
        try:
            self.server_launch()
            if browser is not None:
                webbrowser.get(browser)  # throws exception in case of missing browser
                browser = '"{}"'.format(browser)
            webbrowser_cmd = [sys.executable, '-c',  # use subprocess to avoid web browser error into terminal
                'import webbrowser; webbrowser.get({}).open("http://localhost:{}/funnel")'.format(
                browser, self.server_port)]
            with open(os.devnull, 'w') as pipe:
                proc = subprocess.Popen(webbrowser_cmd, stdout=pipe, stderr=pipe)
            if timeout >= 10:
                print('Server will run for {} (s) or until KeyboardInterrupt.'.format(timeout))
            wait_status = wait_until(exit_test, timeout, 0.1, self.logger, *args)
        except KeyboardInterrupt:
            print('KeyboardInterrupt')
        except Exception as e:
            print(e)
        finally:
            os.chdir(cur_dir)
            try:  # objects may not be defined in case of exception
                self.server_close()
                proc.kill()
                if not wait_status:
                    print('Communication between browser and server failed: '
                        'check that the browser is not running in private mode.')
            except:
                pass


class CORSRequestHandler(SimpleHTTPRequestHandler):
    """Enables to log message on logger and modifies response header."""
    def log_message(self, format, *args):
        try:
            to_send = "{} - - [{}] {}\n".format(
                self.client_address[0],
                self.log_date_time_string(),
                format%args
            )
            self.server.logger.write(to_send.encode('utf8'))
        except ValueError:  # logger closed
            pass
        except Exception as e:
            print(e)

    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin'.encode('utf8'),
            '*'.encode('utf8'))
        self.send_header('Access-Control-Allow-Methods'.encode('utf8'),
            'GET, POST, OPTIONS'.encode('utf8'))
        self.send_header('Access-Control-Allow-Headers'.encode('utf8'),
            'X-Requested-With'.encode('utf8'))
        SimpleHTTPRequestHandler.end_headers(self)

    def send_head(self):
        if (self.server._URL_HTML is not None) and \
           (self.translate_path(self.path).endswith(self.server._URL_HTML)):
            f = io.BytesIO()
            f.write(self.server._STR_HTML.encode('utf8'))
            length = f.tell()
            f.seek(0)
            self.send_response(200)
            self.send_header("Content-type".encode('utf8'), "text/html".encode('utf8'))
            self.send_header("Content-Length".encode('utf8'), str(length).encode('utf8'))
            self.end_headers()
            return f
        else:
            return SimpleHTTPRequestHandler.send_head(self)


def wait_until(somepredicate, timeout, period=0.1, *args, **kwargs):
    """Waits until some predicate is true."""
    must_end = time.time() + timeout
    while time.time() < must_end:
        if somepredicate(*args, **kwargs):
            return True
        time.sleep(period)
    return False


def exit_test(logger, list_files=None):
    content = logger.getvalue().decode('utf8')
    if list_files is not None:
        raw_pattern = 'GET.*?{}.*?200'  # *? for non-greedy search
        for i, l in enumerate(list_files):
            if i == 0:
                pattern = raw_pattern.format(l)
            else:
                pattern = '{}(.*\n)*.*{}'.format(pattern, raw_pattern.format(l))
        return bool(re.search(pattern, content))
    else:
        return False


def plot_funnel(test_dir, title="", browser=None):
    """Plots funnel results stored in test_dir and displays in default browser.

    Args:
        test_dir (str): path of directory where output files are stored
        [title] (str): plot title
        [browser] (str): web browser to use for displaying plot
    """
    list_files = ['reference.csv', 'test.csv', 'errors.csv', 'lowerBound.csv', 'upperBound.csv']
    for f in list_files:
        file_path = os.path.join(test_dir, f)
        assert os.path.isfile(file_path), "No such file: {}".format(file_path)

    content = re.sub('\$TITLE', title, _TEMPLATE_HTML)
    server = MyHTTPServer(('', 0), CORSRequestHandler,
        str_html=content, url_html='funnel', browse_dir=test_dir)
    server.browse(list_files, browser=browser, timeout=5)


_TEMPLATE_HTML = """
<html>
<head>
  <meta charset="utf-8"/>
  <script src="https://cdn.plot.ly/plotly-latest.min.js"></script>
</head>
<body>
  <div id="myDiv" style="height: 100%; width: 100%;" class="plotly-graph-div"></div>
  <script>
    function makeplot() {
        Plotly.d3.csv("http://localhost:$SERVER_PORT/reference.csv", function(data_ref){
            Plotly.d3.csv("http://localhost:$SERVER_PORT/test.csv", function(data_test){
                Plotly.d3.csv("http://localhost:$SERVER_PORT/errors.csv", function(data_err){
                    Plotly.d3.csv("http://localhost:$SERVER_PORT/lowerBound.csv", function(data_low){
                        Plotly.d3.csv("http://localhost:$SERVER_PORT/upperBound.csv", function(data_upp){
                            var data_raw = {
                                'ref': data_ref,
                                'test': data_test,
                                'err': data_err,
                                'low': data_low,
                                'upp': data_upp
                            };
                            processAll(data_raw);
                        });
                    });
                });
            });
        });
    };

    function processAll(dataIn) {
        data = {};
        for(var index in dataIn) {
            data[index] = processData(dataIn[index]);
        }
        makePlotly(data);
    };

    function processData(allRows) {
        var x = [], y = [];
        for (var i=0; i<allRows.length; i++) {
            row = allRows[i];
            x.push(row['x']);
            y.push(row['y']);
        }
        return {x: x, y:y};
    };

    function makePlotly(data){
        var plotDiv = document.getElementById("plot");
        var traces = [
            {
                x: data['err'].x,
                y: data['err'].y,
                name: 'error',
                xaxis: 'x',
                yaxis: 'y2',
            },
            {
                x: data['test'].x,
                y: data['test'].y,
                name: 'test',
            },
            {
                name: 'lower bound',
                x: data['low'].x,
                y: data['low'].y,
                showlegend: false,
                line: {width: 0},
                mode: 'lines'
            },
            {
                x: data['ref'].x,
                y: data['ref'].y,
                name: 'reference',
                fillcolor: 'rgba(68, 68, 68, 0.3)',
                fill: 'tonexty',
            },
            {
                name: 'upper bound',
                x: data['upp'].x,
                y: data['upp'].y,
                showlegend: false,
                fillcolor: 'rgba(68, 68, 68, 0.3)',
                fill: 'tonexty',
                line: {width: 0},
                mode: 'lines'
            },
        ];
        var layout = {
            title: {text: '$TITLE'},
            grid: {rows: 2, columns: 1, subplots: [['xy1'], ['xy2']]},
            xaxis1: {
                ticks: 'outside',
                showline: true,
                zeroline: false,
                title: 'x',
                anchor: 'y1',
            },
            yaxis1: {
                domain: [0.3, 1],
                ticks: 'outside',
                showline: true,
                zeroline: false,
                title: 'y',
            },
            yaxis2: {
                domain: [0, 0.18],
                ticks: 'outside',
                showline: true,
                zeroline: false,
                title: 'error [y]',
            },
        };
        Plotly.newPlot('myDiv', traces, layout, {responsive: true});
    };

    makeplot();
    </script>
</body>
</html>
"""


if __name__ == "__main__":
    import argparse
    import csv

    # Configure the argument parser.
    parser = argparse.ArgumentParser(
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description=textwrap.dedent("""\
            Run funnel binary from terminal.\n
            Output `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`, `test.csv` into the output directory (`./results` by default).
        """),
        epilog=textwrap.dedent("""\
            Note: At least one of the two possible tolerance parameters (atol or rtol) must be defined for each axis.
            Relative tolerance is relative to the range of x or y values.\n
            Typical use from terminal:
            $ python {path to pyfunnel.py} --reference trended.csv --test simulated.csv --atolx 0.002 --atoly 0.002 --output results\n
            Full documentation at https://github.com/lbl-srg/funnel
        """)
    )
    required_named = parser.add_argument_group('required named arguments')

    required_named.add_argument(
        "--reference",
        help="Path of CSV file with reference data",
        required=True
    )
    required_named.add_argument(
        "--test",
        help="Path of CSV file with test data",
        required=True
    )
    parser.add_argument(
        "--output",
        help="Path of directory to store output data",
    )
    parser.add_argument(
        "--atolx",
        type=float,
        help="Absolute tolerance along x axis"
    )
    parser.add_argument(
        "--atoly",
        type=float,
        help="Absolute tolerance along y axis"
    )
    parser.add_argument(
        "--rtolx",
        type=float,
        help="Relative tolerance along x axis"
    )
    parser.add_argument(
        "--rtoly",
        type=float,
        help="Relative tolerance along y axis"
    )

    # Parse the arguments.
    args = parser.parse_args()

    # Check the arguments.
    assert (args.atolx is not None) or (args.rtolx is not None),\
        "At least one of the two possible tolerance parameters (atol or rtol) must be defined for x values."
    assert (args.atoly is not None) or (args.rtoly is not None),\
        "At least one of the two possible tolerance parameters (atol or rtol) must be defined for y values."
    assert os.path.isfile(args.reference),\
        "No such file: {}".format(args.reference)
    assert os.path.isfile(args.test),\
        "No such file: {}".format(args.test)

    # Extract data from files.
    data = dict()
    for s in ('reference', 'test'):
        data[s] = dict(x=[], y=[])
        with open(vars(args)[s]) as csvfile:
            spamreader = csv.reader(csvfile)
            for row in spamreader:
                try:
                    data[s]['x'].append(float(row[0]))
                    data[s]['y'].append(float(row[1]))
                except:
                    pass

    # Call the function.
    rc =  compareAndReport(
        xReference=data['reference']['x'],
        yReference=data['reference']['y'],
        xTest=data['test']['x'],
        yTest=data['test']['y'],
        outputDirectory=args.output,
        atolx=args.atolx,
        atoly=args.atoly,
        rtolx=args.rtolx,
        rtoly=args.rtoly,
    )

    sys.exit(rc)
