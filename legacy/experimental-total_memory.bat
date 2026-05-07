@echo off
REM Change to the directory where your .cpp file lives:
cd /d "C:\Users\alden\Desktop\CK3_Dynasty_Project\ck3_dynasty_tree"

REM Compile with g++ (assuming you have MinGW or similar in your PATH):
g++ -o total_memory.exe total_memory.cpp

echo Finished compiling!
pause
total_memory.exe
