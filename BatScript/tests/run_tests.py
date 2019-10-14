import os
import sys
import subprocess
import argparse

def get_tests_impl(tests, test_paths, root, relroot):
    for filename in os.listdir(root):
        abspath = os.path.join(root, filename)
        relpath = os.path.join(relroot, filename)
        if os.path.isdir(abspath):
            get_tests_impl(tests, test_paths, abspath, relpath)
        else:
            base, ext = os.path.splitext(abspath)
            if ext == '.bat':
                test_paths += [base]
            relbase, relext = os.path.splitext(relpath)
            if relext == '.bat':
                tests += [relbase]
            

def get_tests():
    tests = []
    test_paths = []
    get_tests_impl(tests, test_paths, os.path.dirname(os.path.abspath(__file__)), '')
    return tests, test_paths

def run_tests(tests, test_paths, method=None, compiler_path=None):
    all_passed = True
    for test, test_path in zip(tests, test_paths):
        test_name = os.path.basename(test)
        
        if 'ok-' in test_name:
            kind = 'ok'
        elif 'fail-' in test_name:
            kind = 'fail'
        else:
            print("Invalid test %s. Test name must begin with 'ok-' or 'fail-'" % test)
            continue
        
        try:
            argv = [compiler_path, test_path + '.bat']
            if method != None:
                argv += ['--method', method]
            p = subprocess.Popen(argv, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
            stdout, stderr = p.communicate()
            out = stdout if kind == 'ok' else stderr

            if not os.path.exists(test_path + '.out'):
                print('No expected result file for %s, creating one ...' % test)
                with open(test_path + '.out', 'w') as f:
                    f.write(out)
            else:
                with open(test_path + '.out', 'r') as f:
                    expected = f.read()
                
                failed = False
                if kind == 'ok' and expected != stdout:
                    failed = True
                elif kind == 'fail':
                    expected_lines = expected.split('\n')
                    actual_lines = [line.split('Error: ')[-1] for line in stderr.split('\n')]
                    for i in range(len(expected_lines)):
                        if i >= len(actual_lines):
                            failed = True
                            break
                        if not actual_lines[i] in expected_lines[i]:
                            failed = True
                            break
                
                if failed:
                    all_passed = False
                    print('Test %s ... FAIL' % test)
                    print('Dumping output...')
                    print(out)
                else:
                    print('Test %s ... PASS' % test)
        except Exception as e:
            raise
            sys.stderr.write('Failed! %s' % e.message)
    return all_passed

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--method', type=str, default='vm')
    parser.add_argument('--compiler', type=str, default='BatScript.exe')
    args = parser.parse_args()

    tests, test_paths = get_tests()
    all_passed = run_tests(tests, test_paths, compiler_path=args.compiler, method=args.method)
    if all_passed:
        sys.exit(0)
    else:
        sys.exit(1)

if __name__ == '__main__':
    main()
