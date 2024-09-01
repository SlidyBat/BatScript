class File:
    native constructor(self : File, filename : string, flags : string)
    native read(self : File, size : int = -1) -> string
    native write(self : File, data : string)

    def test(self : File):
        print "Hello from File!"

native open(filename : string, flags : string) -> File
native close(file : File)
