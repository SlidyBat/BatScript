import os
import sys
import subprocess
import argparse

def get_tests_impl(tests, root, relroot):
    for filename in os.listdir(root):
        abspath = os.path.join(root, filename)
        relpath = os.path.join(relroot, filename)
        if os.path.isdir(abspath):
            get_tests_impl(tests, abspath, relpath)
        else:
            base, ext = os.path.splitext(relpath)
            if ext == '.bat':
                tests += [base]

def get_tests():
    tests = []
    get_tests_impl(tests, os.path.dirname(os.path.abspath(__file__)), '')
    return tests

def run_tests(tests, method=None):
    for test in tests:
        test_name = os.path.basename(test)
        
        if 'ok-' in test_name:
            kind = 'ok'
        elif 'fail-' in test_name:
            kind = 'fail'
        else:
            print("Invalid test %s. Test name must begin with 'ok-' or 'fail-'" % test)
            continue
        
        try:
            argv = ['BatScript.exe', test + '.bat']
            if method != None:
                argv += ['--method', method]
            p = subprocess.Popen(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            stdout, stderr = p.communicate()
            out = stdout if kind == 'ok' else stderr

            if not os.path.exists(test + '.out'):
                print('No expected result file for %s, creating one ...' % test)
                with open(test + '.out', 'w') as f:
                    f.write(out)
            else:
                with open(test + '.out', 'r') as f:
                    expected = f.read()
                
                failed = False
                if kind == 'ok' and expected != stdout:
                    failed = True
                elif kind == 'fail' and expected != stderr:
                    failed = True
                
                if failed:
                    print('Test %s ... FAIL' % test)
                    print('Dumping output...')
                    print(out)
                else:
                    print('Test %s ... PASS' % test)
        except Exception as e:
            raise
            sys.stderr.write('Failed! %s' % e.message)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--method', type=str, default='vm')
    args = parser.parse_args()

    tests = get_tests()
    run_tests(tests, method=args.method)

if __name__ == '__main__':
    main()