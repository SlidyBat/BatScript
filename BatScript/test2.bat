//import core
//
//int f = 0
//
//int fib(int n):
//	if n <= 1: return n
//	return fib(n - 2) + fib(n - 1)
//
//var i = 0
//while i < 25:
//	var start = time()
//	var f = fib(i)
//	var time_taken = (time() - start) / 1000.0
//	var result = format("Fib %i = %i (completed in %.3fs)", i, f, time_taken)
//	print result
//	i += 1

// Type inference
var x = [5, 1, 2, 6] // type of x is now int[]

// Dynamic array
int[] y = [5, 3]
//print y[3] // runtime out of bounds error (Array index 3 out of bounds)

print x[-1] // Prints last number "6"
x[-1] = 2 // Sets last number to 2
print x[-1] // Prints "2"
x += 5 // Appends 5 to list
print x[-1] // Prints "5"
// x += "lol" // Compile error (Cannot use operator '+=' on expressions of types int[] and string)

// Re-assignment
x = [2, 3]
print x[0]

// Fixed array
int[100] z
z += 5 // Error, can't expand fixed size array

//// Array slicing (copy)
//int[] y = x[5:8]
//int[5] z = y // error if buffer too small
//
//for i in z:
//	print z
