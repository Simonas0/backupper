# Backupper
A file back-up utility with asynchronous I/O for Linux.

# Features
- Backs up files from one directory into another as they are modified
- Deletes files from both directories, if file is prefixed with "delete_"
- Logs all actions
- Displays and filters logs based on action, filename regex and time
- Preserves filter state between application reboots

# Building
Prerequisites:
- Linux machine
- GCC 11*
- make

\* - any compiler with C++17 support should do the trick, but Makefile might require minor changes (especially `${CC}` variable)

To build, run:

`make`

# Usage
To start backupper, run:

`backupper [hotDir] [bakDir]`

Where [hotDir] and [bakDir] are paths to directories. While backupper is running, you should be able to browse logs using keystrokes defined in brackets ([]).