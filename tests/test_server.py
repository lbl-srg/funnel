#!/usr/bin/env python
# -*- coding: utf-8 -*-
import contextlib
import gc
import http.client
import io
import tempfile
import threading
import time
import unittest
import warnings

from test_import import *

from pyfunnel import CORSRequestHandler, MyHTTPServer

TEST_DIR = os.path.dirname(os.path.abspath(__file__))
PLOT_FILES = ['reference.csv', 'test.csv', 'errors.csv', 'lowerBound.csv', 'upperBound.csv']


def get(port, path):
    """Send GET request and return status, CORS header and body."""
    con = http.client.HTTPConnection('127.0.0.1', port, timeout=5)
    try:
        con.request('GET', path)
        res = con.getresponse()
        return res.status, res.getheader('Access-Control-Allow-Origin'), res.read()
    finally:
        con.close()


def wait_until_closed(server, timeout=5):
    """server_close may close the socket in a separate thread."""
    must_end = time.time() + timeout
    while server.socket.fileno() != -1 and time.time() < must_end:
        time.sleep(0.05)
    return server.socket.fileno() == -1


class TestServerClose(unittest.TestCase):

    def test_bind_failure(self):
        """A server whose port cannot be bound closes silently and leaves no thread."""
        server = MyHTTPServer(('127.0.0.1', 0), CORSRequestHandler, str_html='')
        try:
            threads = set(threading.enumerate())
            out = io.StringIO()
            with warnings.catch_warnings(record=True) as w, contextlib.redirect_stdout(out):
                warnings.simplefilter('always', ResourceWarning)
                # Also checks that SO_REUSEADDR does not allow binding a port in use (Windows).
                with self.assertRaises(OSError):
                    MyHTTPServer(('127.0.0.1', server.server_port), CORSRequestHandler, str_html='')
                gc.collect()
            time.sleep(0.2)
            self.assertEqual(out.getvalue(), '')
            self.assertEqual(set(threading.enumerate()) - threads, set())
            self.assertEqual([str(x.message) for x in w if issubclass(x.category, ResourceWarning)], [])
        finally:
            server.server_close()

    def test_close_not_started(self):
        server = MyHTTPServer(('127.0.0.1', 0), CORSRequestHandler)
        server.server_close()
        self.assertTrue(wait_until_closed(server))

    def test_close_started(self):
        server = MyHTTPServer(('127.0.0.1', 0), CORSRequestHandler, str_html='')
        server.server_launch()
        server.server_close()
        self.assertTrue(wait_until_closed(server))
        server.thread.join(5)
        self.assertFalse(server.thread.is_alive())


class TestRequestHandler(unittest.TestCase):

    def setUp(self):
        self.cur_dir = os.getcwd()
        self.tmp_dir = tempfile.mkdtemp()
        for f in ['allowed.csv', 'secret.txt', os.path.join('dir', 'allowed.csv')]:
            os.makedirs(os.path.join(self.tmp_dir, os.path.dirname(f)), exist_ok=True)
            # No newline translation, so that the served content is the same on Windows.
            with open(os.path.join(self.tmp_dir, f), 'w', newline='') as fh:
                fh.write('SECRET' if f == 'secret.txt' else 'x,y\n')
        # The handler serves the current directory.
        os.chdir(self.tmp_dir)
        self.server = None

    def tearDown(self):
        if self.server is not None:
            self.server.server_close()
            wait_until_closed(self.server)
        os.chdir(self.cur_dir)
        shutil.rmtree(self.tmp_dir, ignore_errors=True)

    def start(self, **kwargs):
        self.server = MyHTTPServer(('127.0.0.1', 0), CORSRequestHandler,
                                   str_html='<p>page</p>', url_html='funnel', **kwargs)
        self.server.server_launch()
        return self.server.server_port

    def assertServed(self, port, path, status, body=None):
        res = get(port, path)
        self.assertEqual(res[0], status, path)
        self.assertIsNone(res[1], 'CORS header for ' + path)
        self.assertNotIn(b'SECRET', res[2], path)
        if body is not None:
            self.assertEqual(res[2], body, path)

    def test_restricted(self):
        port = self.start(allowed_paths=['allowed.csv', 'dir'])
        self.assertServed(port, '/funnel', 200, b'<p>page</p>')
        self.assertServed(port, '/', 200, b'')
        self.assertServed(port, '/allowed.csv', 200, b'x,y\n')
        self.assertServed(port, '/allowed.csv?t=0', 200, b'x,y\n')
        self.assertServed(port, '/dir/allowed.csv', 200, b'x,y\n')
        for path in ['/secret.txt', '/dir/', '/x/funnel', '/dirX/allowed.csv',
                     '/../secret.txt', '/dir/../secret.txt', '/%2e%2e/secret.txt']:
            self.assertServed(port, path, 404)

    def test_restricted_symlink(self):
        try:
            os.symlink(os.path.join(self.tmp_dir, 'secret.txt'),
                       os.path.join(self.tmp_dir, 'dir', 'link.csv'))
        except (OSError, NotImplementedError):
            self.skipTest('Symbolic links not supported')
        port = self.start(allowed_paths=['dir'])
        self.assertServed(port, '/dir/link.csv', 404)

    def test_unrestricted(self):
        """Without allowed_paths, the whole directory is served, but without CORS headers."""
        port = self.start()
        status, cors, body = get(port, '/')
        self.assertEqual(status, 200)
        self.assertIsNone(cors)
        self.assertIn(b'secret.txt', body)
        self.assertEqual(get(port, '/secret.txt')[0], 200)
        self.assertEqual(get(port, '/x/funnel')[2], b'<p>page</p>')


class TestPlotFunnel(unittest.TestCase):

    def test_plot_funnel(self):
        """Simulate the requests of the browser and check that the server shuts down early."""
        info = {}
        server_launch = MyHTTPServer.server_launch

        def client(server):
            status, _, page = get(server.server_port, '/funnel')
            info['page'] = (status, page)
            info['files'] = [get(server.server_port, '/' + f)[0] for f in PLOT_FILES]
            info['root'] = get(server.server_port, '/')[2]

        def launch(server):
            server_launch(server)
            info['address'] = server.server_address[0]
            info['start'] = time.time()
            threading.Thread(target=client, args=(server,), daemon=True).start()

        MyHTTPServer.server_launch = launch
        try:
            with contextlib.redirect_stdout(io.StringIO()):
                # Unknown browser: no browser is launched.
                pyfunnel.plot_funnel(os.path.join(TEST_DIR, 'fail1', 'results'), browser='no-such-browser')
        finally:
            MyHTTPServer.server_launch = server_launch
        duration = time.time() - info['start']

        self.assertEqual(info['address'], '127.0.0.1')
        self.assertEqual(info['page'][0], 200)
        self.assertNotIn(b'localhost', info['page'][1])
        self.assertNotIn(b'SERVER_PORT', info['page'][1])
        self.assertEqual(info['files'], [200] * len(PLOT_FILES))
        # No directory listing: plot_funnel restricts the files served.
        self.assertEqual(info['root'], b'')
        # The default timeout is 10 s.
        self.assertLess(duration, 8)


if __name__ == '__main__':
    unittest.main(verbosity=2)
