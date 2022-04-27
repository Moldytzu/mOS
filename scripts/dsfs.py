import os, sys

inputFolder = ''
outputFile = ''

def writeEntry(name):
    global inputFolder, outputFile

    size = os.stat(inputFolder + name).st_size # get file size
    contents = open(inputFolder + name,"rb") # open input file
    name = name + ('\0' * (56 - len(name)))  # append nulls so the len is 56
    
    # write the entry
    outputFile.write(name.encode('ascii')) # name 
    outputFile.write(size.to_bytes(8, 'little')) # size
    outputFile.write(contents.read()) # contents

    contents.close() # close the input file

def build():
    global inputFolder, outputFile

    # get the files from the input folder
    files = os.listdir(inputFolder)

    # write the header
    outputFile = open(outputFile, "wb")
    outputFile.write(b'DS')  # signature
    outputFile.write(0x10.to_bytes(2, 'little'))
    outputFile.write(len(files).to_bytes(4, 'little'))  # entries count

    # write the entries
    for file in files:
        writeEntry(file)

    # close the output file
    outputFile.close()

if __name__ == "__main__":
    if len(sys.argv) != 3:  # display usage if the required parameters aren't supplied
        print(f"python3 {sys.argv[0]} <input folder> <output file>")
        exit(-1)

    # set the global variables to the parameters
    inputFolder = sys.argv[1]
    outputFile = sys.argv[2]

    # start building the filesystem
    build()
