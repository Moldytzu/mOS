# Damn Simple RAM Filesystem (rev. 1.0)
## Header
#### It is the first 8 bytes of the filesystem. It is an ordered packed structure that holds the following data:
> 2 byte string - signature (always 'DS')
> 2 byte (little-endian) unsigned integer - version (always 0x10)
> 4 byte (little-endian) unsigned integer - entries count
## Entry
#### It is an ordered packed structure that holds the following data:
> 52 byte string - name
> 4 byte (little-endian) unsigned integer - parent's entry index
> 8 byte (little-endian) unsigned integer - size of the contents in bytes