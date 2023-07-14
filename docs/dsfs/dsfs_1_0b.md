# Damn Simple RAM Filesystem (rev. 1.0b)

### Changes from last revision
-   Define an end for the chain of entries
-   Fix spelling

## Note before
All integers explained here are encoded in little-endian order.

## Definitions
| Term | Explanation |
| - | - |
| DSFS | abbreviation for "Damn Simple RAM Filesystem" |
| string | an array of 8-bit unsigned integers that can be null-terminated |
| 'x' | a character encoded in ASCII (in this example is x) |
| "xy" | a non-terminated string (in this example it holds x and y)|
| chain of entries | the totality of all entries in a DSFS filesystem |

## Structures
### Header
It is an 8-byte ordered packed structure that holds the following data:
- 2 bytes string - signature (always "DS")
- 2 bytes (little-endian) unsigned integer - DSFS version (always 0x10)
- 4 bytes (little-endian) unsigned integer - entries count (stores count of entries in the filesystem)

### Entry marker
It is used as a header that marks the start of a file's contents. The file's contents starts at the very next byte after this. It is an ordered packed structure that holds the following data:
- 56 byte null-terminated string - name
- 8 byte (little-endian) unsigned integer - size of the contents in bytes

## Layout
A chain of entries must start with the header and end with an entry marker with size 0 and name consisting of 56 zeros. The header's entries field mustn't include the last entry.
### Example
#### Header: signature: "DS"; version: 0x10; entries: 2
#### Entry marker: name: "test" + 52 zeros; size: 15
This is a test!
#### Entry marker: name: "1234" + 52 zeros; size: 12
numbers 1234
#### Entry marker: name: "56 zeros; size: 0