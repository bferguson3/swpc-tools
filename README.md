# Sword World PC Translation Tool v0.2
### and other files

## swpctool
<img width="813" alt="Screenshot 2025-05-10 at 10 24 34 PM" src="https://github.com/user-attachments/assets/3fd31f8a-0103-4a8a-b14c-9937887f28b5" />


## How to use
- File -> Open to load db file
- << and >> navigate to next and previous line
- Commit will save changes on ALL PAGES to the local db. If you navigate away without committing, your changes will not be lost. Commit will not work if the byte size of each string does not match. 
- Mark Bad will flag the string as not having any Japanese text in it, and reinsertion will be skipped. (DAT only)
- Mark complete will hide this word from subsequent clicks on << and >> (DAT only)
- Sizes of both strings MUST MATCH. This is the SJIS equivalent byte size!
- Goto string allows you to select an individual string by index.
- number of duplicates indicates how many similar lines will be reinserted later. (DAT only)
- Search searches through the current ARF bytes for the equivalent string (ARF only)

KB Shortcuts:
- Ctrl+Right/Left : Next / previous word
- Ctrl+Enter : Commit all changes
- Ctrl+G : Goto
- Ctrl+B : Mark bad
- Ctrl+C : Mark complete
- Ctrl+I : Insert mode (warning: bugged on windows)

Version History<br>
v0.37 - Added search function<br>
v0.31-0.36 - Bugfixes<br>
v0.3 - Added ARF support<br>
v0.2 - Fixed % display, duplicate count, added kb shortcuts<br>
- bugfix: no more crash when go to a non-number or blank string #<br>
- bugfix: goto now goes to appropriate index instead of index + 1<br>
v0.1 - Initial Release<br>

### build instructions
depends upon wxWidgets and ICU<br>
place this repo in wxWidgets/build-folder/samples/ <br>
then adjust paths to ICU etc in the Makefile and then <br>

`$ make`

## swpcex.py 
## WARNING: The python tool does not generate a proper translation file on Windows. 
## Only use the translation.dat provided or run it from Linux or OSX.
Usage:

`$ python3 ./swpc-eng.py disk.d88`

This will create a translation.dat file which contains all found words. <br>
Note this tool must be modified internally for use with other disks. 
