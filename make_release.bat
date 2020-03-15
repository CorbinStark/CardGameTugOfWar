@echo off

cls
cl /EHsc -Iinclude main.cpp *.cpp ENGINE\*.c ENGINE\*.cpp libs\*.lib msvcrt.lib shell32.lib user32.lib gdi32.lib opengl32.lib
del *.obj
del *.pdb
del *.ilk
del CardGame.exe
ren main.exe CardGame.exe
SRPG.exe