# Build information
This is a step by step guide on how to build on the following supported platforms:

## Debian / Ubuntu / WSL2 Ubuntu (recommended for Windows users)

```
sudo apt install libncursesw5-dev
make
```

Yes, that's it. Program will be located in `./bin/release/game`

## Build as native Windows executable (Msys2-mingw64) (advanced)
Oh boy. I recommended WSL2 for a reason.

1. Install Msys2 and open Msys2-mingw64
2. Update system and install needed dependencies:
```
pacman -Syu
pacman -S git make mingw-w64-x86_64-toolchain mingw-w64-x86_64-gcc mingw-w64-x86_64-make
```
3. Download and build PDCurses with widechar support:
```
wget https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.tar.gz
tar xvf 3.9.tar.gz
cd PDCurses-3.9/wincon/
mingw32-make WIDE=Y
```
4. Go back to the project directory, and build with including/linking PDCurses:
```
# If you downloaded PDCurses to another directory, set it to that instead
make EXTRA_INCLUDES=-I/home/$USER/PDCurses-3.9/ EXTRA_LDFLAGS=-L/home/$USER/PDCurses-3.9/
```

After following these steps, your program will be located at `./bin/release/game.exe`

## Build as native Windows executable (MSVC) (unstable)

1. Install Visual Studio 2022 and C++ components, then open **Developer Powershell for VS 2022**

2. Download, extract and build PDCurses with widechar support
```
Invoke-WebRequest https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.zip -OutFile .\PDCurses-3.9.zip
Expand-Archive .\PDCurses-3.9.zip .
cd .\PDCurses-3.9\wincon
nmake.exe -f .\Makefile.vc WIDE=Y
```
3. Run Makefile.vc using cmake
```
nmake.exe -f .\Makefile.vc EXTRA_INCLUDES="/IC:\Users\diego\src\PDCurses-3.9\" LIB_PATH="C:\Users\diego\src\PDCurses-3.9\wincon\"
```

If this works, the compiled game will be at `.\bin\release\game.exe`
