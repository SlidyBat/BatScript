g := 0

def f():
	g += 1
	return g

def foo(a : int, b : int, c : int):
	print a
	print b
	print c

foo(f(), f(), f())