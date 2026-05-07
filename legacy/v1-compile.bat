@echo off
REM Change to the directory where your .cpp file lives:
cd /d "C:\Users\Alden\Desktop\paradox_converters\ck3_dynasty_tree"

REM Compile with g++ (assuming you have MinGW or similar in your PATH):
g++ -o character_parser.exe character_parser.cpp

echo Finished compiling!
pause
