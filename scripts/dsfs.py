#!/usr/bin/python3
# utility to create a dsfs 1.0b image file from a folder

import os
import sys

inputFolder = ''
outputFile = ''
    
def appendEntryMarker(name, size):
    name = name + ('\0' * (56 - len(name)))  # append 0s so the len is 56
    outputFile.write(name.encode('ascii'))  # name
    outputFile.write(size.to_bytes(8, 'little'))  # size

def appendContents(contents):
    outputFile.write(contents)

def appendFile(name):
    global inputFolder, outputFile

    size = os.stat(inputFolder + name).st_size  # get file size
    contents = open(inputFolder + name, "rb")  # open input file

    appendEntryMarker(name, size) # append the entry marker
    appendContents(contents.read()) # append file contents

    contents.close()  # close the input file

def build():
    global inputFolder, outputFile

    # get the files from the input folder
    files = os.listdir(inputFolder)
    
    print(f"dsfs: preparing to build a dsfs image with {len(files)} entries")

    # write the header
    outputFile = open(outputFile, "wb")
    outputFile.write(b'DS')  # signature
    outputFile.write(0x10.to_bytes(2, 'little'))
    outputFile.write(len(files).to_bytes(4, 'little'))  # entries count

    # write the entries
    for file in files:
        appendFile(file)

    appendEntryMarker("", 0) # terminate the chain of entries

    # close the output file
    outputFile.close()

    print("dsfs: done. thank you")

if __name__ == "__main__":
    if len(sys.argv) != 3:  # display usage if the required parameters aren't supplied
        print(f"{sys.argv[0]} <input folder> <output file>")
        exit(-1)

    # set the global variables to the parameters
    inputFolder = sys.argv[1]
    outputFile = sys.argv[2]

    # start building the filesystem
    build()
