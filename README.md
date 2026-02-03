# IncrementalGame
A cli-based incremental game

## Build Requirements
- g++ >= 14.1
- libncursesw5-dev (Linux)
- PDCurses, built with widechar support (Windows) (See BUILDING)

## Runtime Requirements 
- libncursesw5 (Linux)

## Terminal Requirements
- 256-bit colors
- wchar rendering support

# Building

## On Linux
Run the following commands
```
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## On Windows (MSVC)
1. Install Visual Studio 2022 and C++ components, then open Deverloper Powershell for VS 2022

2. Build PDCurses with widechar enabled:
```
git clone https://github.com/wmcbrine/PDCurses.git
cd .\PDCurses\wincon
nmake.exe -f .\Makefile.vc WIDE=Y
```

3. Build this project: 
```
New-Item -Type Directory .\build
cd .\build
cmake .. -DCMAKE_PREFIX_PATH="C:/Path/To/PDCurses;C:/Path/To/PDCurses/wincon"
cmake --build . --config Release
```

(Note: Replace both paths of CMAKE_PREFIX_PATH with the location of the cloned PDCurses/ and PDCurses/wincon/)

