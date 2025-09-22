#!/usr/bin/env python
# -*- coding: utf-8 -*-
from test_import import *

if __name__ == "__main__":
    cmake_test_dir = sys.argv[1]

    # Batch mode if running on Travis or -B option from terminal
    is_travis = 'TRAVIS' in os.environ
    batch = is_travis
    try:
        argv_2 = sys.argv[2]
        if argv_2 == '-B':
            batch = True
    except:
        pass

    print('Cleaning {} and summarizing numerics tests results.'.format(cmake_test_dir))
    dif_test = 0

    for d in next(os.walk(cmake_test_dir))[1]:
        if 'test_numerics_' not in d:
            continue

        try:
            with open(os.path.join(cmake_test_dir, d, 'test_log.json'), 'r') as f:
                test_log = json.load(f)

            dif_err = test_log['dif_err']

            if max(dif_err) != 0:
                dif_test += 1

                if not batch:
                    print(
"""Test {} with data from {} failed on:
{:%} x points and {:%} y points.""".format(test_log['test_name'], test_log['test_dir'], dif_err[0], dif_err[1]))
                    # pyfunnel.plot_funnel(os.path.join(test_log['test_dir'], 'results'), title='Original')
                    # pyfunnel.plot_funnel(os.path.join(test_log['tmp_dir'], 'results'), title='New')

                    replace = input(
"""Do you want to keep new results from {} and replace results stored in {}?
Yes (y) or no (n):""".format(test_log['test_name'], test_log['test_dir']))

                    if re.match('y', replace,re.I):
                        file_names = ['errors.csv', 'lowerBound.csv', 'reference.csv', 'test.csv', 'upperBound.csv']
                        for f in file_names:
                            file_path = os.path.join(test_log['tmp_dir'], 'results', f)
                            try:
                                shutil.move(file_path, os.path.join(test_log['test_dir'], 'results', f))
                                print("File {} updated.".format(f))
                            except Exception as e:
                                print("File {} could not be updated because: {}".format(f, e))
                    else:
                        print("No file updated.")
        except:
            pass
        finally:
            shutil.rmtree(os.path.join(cmake_test_dir, d))

    if dif_test == 0:
        print('All numerics tests passed.')
    else:
        print('{} numerics tests failed.'.format(dif_test))
