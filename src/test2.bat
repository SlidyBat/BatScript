// Hello world!
from file import File

def find(s: string, needle: string) -> int:
    i := 0
    while true:
        if s[i] == needle:
            return i
        i += 1
    return 0

f := File("test2.bat", "r")
data := f.read(-1)
line_end := find(data, '\n')
print(data[3:line_end])