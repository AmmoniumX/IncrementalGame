# Build information
This is a step by step guide on how to build on the following supported platforms:

## Debian / Ubuntu / WSL2 Ubuntu (recommended for Windows users)

```
sudo apt install libncursesw5-dev
make
```

Yes, that's it. Program will be located in `./bin/release/game`

## Build as native Windows executable (advanced)
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