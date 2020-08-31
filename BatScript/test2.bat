g := 0

def f():
	g += 1
	return g

def f2():
	g += 1

a := f()
f2()
b := f()

print a
print b