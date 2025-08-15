These are the steps that I followed in order to compile my program for windows using Microsoft Visual Studio 2022 and its packet manager (vcpkg).

# How to compile for windows (dynamically)
First of all, we are going to install **Visual Studio 2022** and 
the **vcpkg** packet manager, which have a nice integration. 
To install the latter, you will need to install [**git for windows**](https://git-scm.com/download/win), 
I followed a very good [tutorial](https://learn.microsoft.com/es-es/vcpkg/get_started/get-started-msbuild?pivots=shell-cmd) 
by microsoft which explains step by step 
how to install vcpkg and configure it to work beside visual studio, however for this we will only need point nº1 of the tutorial.

Before continuing, you must add vcpkg to your path (or it is recommended to do so at least). Do it by using: 

 `Right-click on windows icon > System > Advanced System Configuration > Environment Variables > Double-click Path` 
  
  Then you can simply copy the path to your vcpkg installation (folder where vcpkg.exe is found) into the last row

After having everything installed, we can follow the next steps:

1. Create a new project in visual studio using the `Console App` template and name it however you want. It is important to specify that we want the solution and the project in the same directory
2. Once inside the project, you can erase the contents of the initial .cpp file (which is just a hello world program) and write/paste your code that uses SFML.
3. Now you can go to `Tools > Command Line > Developer Command Line` in visual studio
4. Run the following commands:
- `vcpkg new --application`
- `vcpkg add port sfml`
  What each one of these commands does is explained in the vcpkg install tutorial mentioned above (part nº2)
5. Now you can close the console and go to the project's properties in visual studio, where you must do the following:
- `Configuration properties > vcpkg > use vcpkg manifest` set it to yes

Now your project is ready to be compiled dynamically (to execute the .exe, the dll files must be in the same folder)

# How to compile the SFML file statically

In case that you want to link the libraries statically (.exe file weighs more, but only the exe needed to run the program), there are a few additional steps to take:
1. In the project's properties in visual studio, go to `Configuration properties > vcpkg` :
- Set the `use static libraries` option to yes
- [optional] Set the `vcpkg configuration` option to debug or release according to your preferences
2. Then go to `Configuration properties > C/C++ > Preprocessor > Preprocessor definitions` and click on edit. Then at the end add the following flag: `SFML_STATIC`
3. Finally, go to `Configuration properties > Linker > Input > Additional dependencies` and click on edit. At the end you must add the following elements at the end depending on what sfml libraries you are using:
- SFML Graphics: `opengl32.lib` and `freetype.lib`
- SFML Window: `opengl32.lib`, `winmm.lib` and `gdi32.lib`
- SFML System: `winmm.lib`
> They don't need to be repeated, it is enough with adding each one a single time. They must each be added on their own in a new line. You can visit [this](https://www.sfml-dev.org/tutorials/3.0/getting-started/visual-studio/#creating-and-configuring-an-sfml-project) page to see a full list of dependencies for all of the SFML libraries

Now your project can be compiled statically!

# How to remove the console from the SFML program

For the final part, because most programs that use SFML are games, and having a black console behind a game does not inspire trust in a game, the following are some steps to prevent the console from executing when your program (.exe file) runs:

1. Inside visual studio, go to your project's properties and then set `Configuration properties > Linker > System > Subsystem` to the option `Windows (/SUBSYSTEM:WINDOWS)`
2. Because doing only the step above will result in a compilation error, you must now change your program's main function name from `int main(int argc, char *argv[])...` to `int WinMain(int argc, char *argv[])...`

After doing these steps, the program can be compiled without a command window popping up in your game's background

>Take into account that these last two steps are not mandatory (e.g. You can link dynamically and remove the console window without linking statically,
> you can just link the program statically, without removing the console etc.)

