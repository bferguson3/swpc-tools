# Sword World PC Translation Tool
### and other files

## swpctool
![image](https://github.com/user-attachments/assets/a7bb0bcc-d932-41a1-b451-ac5bfb481022)

## How to use
- File -> Open to load db file
- << and >> navigate to next and previous line
- Commit will save changes on ALL PAGES to the local db. If you navigate away without committing, your changes will not be lost. Commit will not work if the byte size of each string does not match. 
- Mark Bad will flag the string as not having any Japanese text in it, and reinsertion will be skipped.
- Mark complete will hide this word from subsequent clicks on << and >>
- Sizes of both strings MUST MATCH. This is the SJIS equivalent byte size!
- Goto string allows you to select an individual string by index.
- number of duplicates indicates how many similar lines will be reinserted later.


depends upon wxWidgets and ICU
place this repo in wxWidgets/build-folder/samples/ then

`$ make`

## swpc-eng.py 
## WARNING: The python tool does not generate a proper translation file. 
## Only use the translation.dat provided or run it from Linux or OSX.
Usage:

`$ python3 ./swpc-eng.py all.d88 disks.d88 to.d88 load.d88`

The first time it is run, a translation database will be built (`translation.dat`). 
This file will eventually be loaded into the tool for translation work. 

