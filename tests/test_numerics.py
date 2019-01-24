from test_import import *

if __name__ == "__main__":
    test_dir = sys.argv[1]
    rc = run_pyfunnel(test_dir)

    assert rc == 0, "Binary status code for {}: {}.".format(test_dir, rc)

    dif_err = dif_test(test_dir)

    assert max(dif_err) == 0,\
        "Number of discrepancies between current and reference error values ({}): {} on x, {} on y.".format(
            test_dir, dif_err[0], dif_err[1])