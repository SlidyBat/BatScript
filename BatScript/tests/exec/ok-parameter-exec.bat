var g = 0

def f():
	g += 1
	return g

def foo(int a, int b, int c):
	print a
	print b
	print c

foo(f(), f(), f())