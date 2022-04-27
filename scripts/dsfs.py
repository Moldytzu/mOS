import os, sys

inputFolder = ''
outputFile = ''

def build():
    global inputFolder, outputFile
    # get the files from the input folder
    files = os.listdir(inputFolder)

    print(f"building filesystem from {files}")

    # write the header
    outputFile = open(outputFile,"wb")
    outputFile.write(b'DS') # signature
    outputFile.write(0x10.to_bytes(2,'little'))
    outputFile.write(len(files).to_bytes(4,'little')) # entries count

if __name__ == "__main__":
    if len(sys.argv) != 3: # display usage if the required parameters aren't supplied
        print(f"python3 {sys.argv[0]} <input folder> <output file>")
        exit(-1)

    # set the global variables to the parameters
    inputFolder = sys.argv[1]
    outputFile = sys.argv[2]
    
    # start building the filesystem
    build()
