var g = 0

def f():
	g += 1
	return g

def f2():
	g += 1

var a = f()
f2()
var b = f()

print a
print b