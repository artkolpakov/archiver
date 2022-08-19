This program serves as an archiver or unarchiver of text files/directories.

To compile use: gcc -std=c99 -o archiver archiver.c

----------------------------------------------------

Use this syntax for archivation of directories/files:
----------------------------------------------------

archiver filename1 filename2 filename3 ... (as many files as you want to archive) filenameN result.arch

This way you tell the program to archive all N files into the same result.arch file.

----------------------------------------------------

The exact same syntax works for archiving the entire directories (and their text file contents):

archiver dirname1 dirname2 dirname3 ... (as many directories as you want to archive) dirnameN result.arch

This way you tell the program to archive all N directories into the same result.arch file.

----------------------------------------------------

Finally, you can also archive multiple files and directories at the same time:

archiver dirname1 dirname2 filename1 dirname3 ... (as many directories and files as you want to archive) dirnameN filenameN result.arch

This way you tell the program to archive all N directories and N files into the same result.arch file.

----------------------------------------------------

Use this syntax for unarchivation of directories/files:
----------------------------------------------------

archiver archiveFile.arch

This way you tell the program to unarchive (unpack and create) all directies and/or files from archiveFile.arch file.

----------------------------------------------------

To test the program, I would recommend first packing multiple files and directories, and then unpacking them (e.g., archiver dirname result.arch and then archiver result.arch).

The implementation details are documented directly in the archiver.c file.
