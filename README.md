# SRIX4K-Reader
SRIX4K Reader is a totally new program to manage SRIX4K NFC tags, which follows the guidelines imposed by the ST datasheet.
This program supports SRIX4K and ST25TB04K tags, and it's able to communicate with every NFC reader/writer supported by "libnfc" library, including the famous PN532.

## Features
- Automatic read of written blocks, to check data consistency.
- uint32 as internal data type.
- Reader functions separated by logic SRIX, so the library could be changed in the future.
- Logic representation of SRIX4K has separated EEPROM sections, to set different permissions and define a write-order.

## Build
Requires [libnfc](https://github.com/nfc-tools/libnfc) installed in your pc.
```
mkdir build
cd build
cmake ..
make
```

## Usage
```
Usage: ./SRIX4K-Reader [-h] [-p] [-r file] [-w file] [-c] [-o]

Options:
  -h        show this help message
  -p        print information about NFC tag
  -r file   read eeprom from a file, if not present read from NFC tag
  -w file   write eeprom to a file
  -c        write changes to NFC tag eeprom
  -o        reset SRIX4K OTP blocks
```

## Warning
Every feature hasn't been fully tested and could create problems, I do not take any responsibility in case of damage to your NFC tags.