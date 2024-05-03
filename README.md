# SFML Minesweeper

This is a version of the minesweeper game that I made
on my own in order to practice using C++ classes and the SFML
library.
 
For this game I tried to inspire myself on a 'beach' landscape
instead of making your normal gray background simple af minesweeper, 
it also has a difficulty selector screen in order to change the size
of the mine matrix that is being generated, as well as various sprites 
that are taken out of a same tileset texture, which makes reading the 
code good practice for learning the SFML library.
 
Feel free to download and modify the code as you wish.

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
>

---
# Game images
![imagen](https://github.com/iikerm/sfml-minesweeper/assets/151840754/767fc612-1a08-4870-a5e2-3fd47974e518)

![imagen](https://github.com/iikerm/sfml-minesweeper/assets/151840754/6844eb85-fab9-44f1-8dab-7bbb8b9d8bb1)
