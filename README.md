# Sword World PC Translation Tool v0.2
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

KB Shortcuts:
- Ctrl+Right/Left : Next / previous word
- Ctrl+Enter : Commit all changes
- Ctrl+G : Goto
- Ctrl+B : Mark bad
- Ctrl+C : Mark complete

Version History<br>
v0.2 - Fixed % display, duplicate count, added kb shortcuts<br>
v0.1 - Initial Release<br>

depends upon wxWidgets and ICU<br>
place this repo in wxWidgets/build-folder/samples/ then

`$ make`

## swpcex.py 
## WARNING: The python tool does not generate a proper translation file on Windows. 
## Only use the translation.dat provided or run it from Linux or OSX.
Usage:

`$ python3 ./swpc-eng.py disk.d88`

This will create a translation.dat file which contains all found words. <br>
Note this tool must be modified internally for use with other disks. 
