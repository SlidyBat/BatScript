import core

int fib(int n):
	if n <= 1: return n
	return fib(n - 2) + fib(n - 1)

var i = 0
while i < 25:
	var start = time()
	var f = fib(i)
	var time_taken = (time() - start) / 1000.0
	var result = format("Fib %i = %i (completed in %.3fs)", i, f, time_taken)
	print result
	i += 1