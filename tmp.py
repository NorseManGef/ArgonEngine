# type:ignore
from os import listdir
from os.path import isfile, join

def walk_dir(directory):
    dirs = []
    for file in listdir(directory):
        fullpath = join(directory, file)
        if isfile(fullpath):
            if file.endswith(".cpp") or file.endswith(".c"):
                yield fullpath.replace("\\", "/")
        else:
            dirs.append(fullpath)
    
    for dir in dirs:
        yield from walk_dir(dir)


for f in walk_dir("android"):
    print(f)