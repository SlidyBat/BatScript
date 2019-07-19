var a = "global"

def test():
	def showA(): print a
	
	showA()
	var a = "local"
	showA()

test()