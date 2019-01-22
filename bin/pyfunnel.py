#!/usr/bin/env python
#######################################################
# Python binding for funnel library: compareAndReport
# Function that plots results: plot_funnel
#######################################################
from __future__ import absolute_import, division, print_function, unicode_literals

import os
import sys
import platform
import six
import webbrowser
import re
import time
import threading
from ctypes import cdll, POINTER, c_double, c_int, c_char_p, py_object, pythonapi, c_long
try:
    from http.server import HTTPServer, SimpleHTTPRequestHandler # Python 3
except ImportError:
    from SimpleHTTPServer import BaseHTTPServer
    HTTPServer = BaseHTTPServer.HTTPServer
    from SimpleHTTPServer import SimpleHTTPRequestHandler # Python 2


def get_lib_path(project_name):

    """
    Guess the library absolute path. 

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
    
    return lib_path


def compareAndReport(
    xReference,
    yReference,
    xTest,
    yTest,
    outputDirectory,
    atolx=None,
    atoly=None,
    rtolx=None,
    rtoly=None):
    """
    Call funnel binary with list-like objects as x, y reference and test values.
    Output `errors.csv`, `lowerBound.csv`, `upperBound.csv`, `reference.csv`, `test.csv` 
    into the output directory (`./results` by default).
    Note: at least one absolute or relative tolerance parameter must be provided for each axis.

    Args:
        xReference (list-like): x reference values
        yReference (list-like): y reference values
        xTest (list-like): x test values
        yTest (list-like): y test values
        outputDirectory (str): path of directory to store output files
        atolx (float): absolute tolerance along x axis
        atoly (float): absolute tolerance along y axis
        rtolx (float): relative tolerance along x axis
        rtoly (float): relative tolerance along y axis

    Returns:
        None
    """
    # Check arguments.
    ## Logic
    assert (atolx is not None) or (rtolx is not None),\
        "At least one of the two possible tolerance parameters (atol or rtol) must be defined for x values."
    assert (atoly is not None) or (rtoly is not None),\
        "At least one of the two possible tolerance parameters (atol or rtol) must be defined for y values."
    ## Type
    assert isinstance(outputDirectory, six.string_types),\
        "Path of output directory is not a string type." 
    ## Value
    assert len(xReference) == len(yReference),\
        "xReference and yReference must have the same length."
    assert len(xTest) == len(yTest),\
        "xTest and yTest must have the same length."

    # Convert arrays to lists (to support np.array and pd.Series).
    try:
        xReference = list(xReference)
        yReference = list(yReference)
        xTest = list(xTest)
        yTest = list(yTest)
    except Exception as e:
        raise TypeError("Input data arrays could not be converted to lists: {}".format(e))

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

    # Load library.
    try:
        lib_path = get_lib_path('funnel')
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
            tol['rtoly'])
    except Exception as e:
        raise RuntimeError("Library call raises exception: {}".format(e))

    sys.stderr.flush()
    assert retVal == 0, "Binary status code is: {}.".format(retVal)

    return retVal


def plot_funnel(test_dir, browser=None):
    """
    Plot funnel results stored in test_dir. Display plot in default browser.
    Note: On Linux with Chrome as default browser, if there is no existing Chrome window open at 
    function call, an error log is output to the terminal. 
    Use option `browser="firefox"` or `browser="safari"` as a workaround if needed.

    Args:
        test_dir (str): path of directory where output files are stored
        browser (str): (optional) web browser to use for displaying plot

    Returns:
        None

    @todo:
        HTTPServer class extension with __init__ to specify STDERR handle (hard coded in current version)
    """

    list_files = ['reference.csv', 'test.csv', 'errors.csv', 'lowerBound.csv', 'upperBound.csv']
    for f in list_files:
        file_path = os.path.join(test_dir, f)
        assert os.path.isfile(file_path), "No such file: {}".format(file_path)

    class CORSRequestHandler(SimpleHTTPRequestHandler):
        def log_message(self, format, *args):
            func_stdout.write("%s - - [%s] %s\n" %
                         (self.client_address[0],
                          self.log_date_time_string(),
                          format%args))        

        def end_headers(self):
            self.send_header('Access-Control-Allow-Origin', '*')
            SimpleHTTPRequestHandler.end_headers(self)

    # based on https://stackoverflow.com/a/2785908/1056345                                                                                                                                                                                                                                                                         
    def wait_until(somepredicate, timeout, period=0.1, *args, **kwargs):
        must_end = time.time() + timeout
        while time.time() < must_end:
            if somepredicate(*args, **kwargs):
                return True
            time.sleep(period)
        return False

    
    def exit_test(handler, list_files):
        handler.seek(0)
        content = handler.read()
        raw_pattern = 'GET.*?{}.*?200'  # *? for non-greedy search
        for i, l in enumerate(list_files):
            if i == 0:
                pattern = raw_pattern.format(l)
            else:
                pattern = '{}.*\n.*{}'.format(pattern, raw_pattern.format(l))
        return bool(re.search(pattern, content))
    
    def clean():
        threadd = threading.Thread(target=server.shutdown)  # makes main thread stall if no threading
        threadd.daemon = True  # daemonic thread objects are terminated as soon as the main thread exits
        threadd.start()
        server.server_close()
        try:
            func_stdout.close()  
            os.chmod('foo.log', 0o777)
            os.remove('foo.log')
        except Exception as e:
            print('Could not clean properly because: {}'.format(e))
            pass
        finally:
            os.chdir(cur_dir)

    cur_dir = os.getcwd()
    os.chdir(test_dir)
    if os.path.isfile('foo.log'):
        os.chmod('foo.log', 0o777)
        os.remove('foo.log')
    func_stdout = os.fdopen(os.open('foo.log', os.O_CREAT | os.O_RDWR, 0), 'r+', 0)
    
    try:
        server = HTTPServer(('', 0), CORSRequestHandler)
        thread = threading.Thread(target=server.serve_forever)  # multiprocessing.Process yields class pickle error on Windows
        thread.daemon = True  # daemonic thread objects are terminated as soon as the main thread exits
        thread.start()
        content = re.sub('\$SERVER_PORT', str(server.server_port), template_html)
        with open('plot.html', 'w') as f:
            f.write(content)
        webb = webbrowser.get(browser)
        webb.open_new_tab('plot.html')  # Chrome bug (works but terminal log) if no existing window, Firefox OK
        wait_until(exit_test, 5, 0.1, func_stdout, list_files)
    except Exception as e:
        print('Went wrong: {}'.format(e))
        pass
    finally:
        clean()

    return


template_html = """
<html>
<head>
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
            margin: {pad: 15}
        };
        Plotly.newPlot('myDiv', traces, layout);
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
    parser = argparse.ArgumentParser(description=
    """Run funnel library from terminal.
    Note: At least one of the two possible tolerance parameters (atol or rtol) must be defined for x values.
    """
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
        help="Absolute tolerance in x direction"
    )
    parser.add_argument(
        "--atoly",
        type=float,
        help="Absolute tolerance in y direction"
    )
    parser.add_argument(
        "--rtolx",
        type=float,
        help="Relative tolerance in x direction"
    )
    parser.add_argument(
        "--rtoly",
        type=float,
        help="Relative tolerance in y direction"
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
    if args.output is None:
        print("Output directory not specified: results are stored in subdirectory `results` by default.")
        args.output = "results"
    assert isinstance(args.output, six.string_types),\
        "Path of output directory is not a string type."    

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

    