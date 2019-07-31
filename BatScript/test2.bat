import core

var a = 10

int fib(int n):
	if n <= 1: return n
	return fib(n - 2) + fib(n - 1)

var start = time()
print fib(a)
print time() - start