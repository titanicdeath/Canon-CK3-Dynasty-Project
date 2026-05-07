@echo off
REM Change to the directory where your .cpp file lives:
cd /d "C:\Users\alden\Desktop\CK3_Dynasty_Project\ck3_dynasty_tree"

REM Compile with g++ (assuming you have MinGW or similar in your PATH):
g++ -o new_parser.exe new_parser.cpp

echo Finished compiling!
pause
@echo on
new_parser.exe
