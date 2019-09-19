import core

int fib(int n):
    if n < 2: return n
    return fib(n - 2) + fib(n - 1)

var start = time()
print fib(25)
print (time() - start) / 1000.0