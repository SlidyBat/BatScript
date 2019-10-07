var g = 0

void test

def f():
	g += 1
	return g

def foo(int a, int b):
	print a
	print b

foo(f(), f())