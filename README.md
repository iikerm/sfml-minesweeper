# sfml-minesweeper

This is a version of the famous minesweeper game that I made
on my own in order to practice using C++ classes and the SFML
library. Feel free to download and modify the code as you wish

---
# Compiling on Ubuntu
To compile the code on ubuntu (and probably on other linux distros
as well), you first must have installed the `libsfml-dev` package:
 
`sudo apt install libsfml-dev`

After that, you can compile the file `minesweeper.cc` as you would
with any other SFML C++ file:

`g++ minesweeper.cc -o minesweeper -lsfml-window -lsfml-system -lsfml-graphics`

> In this case, the program only needs the SFML graphics header file, but
> that also requires to include both the system and the window header files
