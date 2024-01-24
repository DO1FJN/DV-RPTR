@echo off
if %1!==! goto message

echo Erasing and Programming Bootloader....
avr32program -c USB:00A200006E88 -pavrdragon --part UC3B0256 program -finternal@0x80000000 -cint -F bin -O0x80000000 -E -u -v %1
ping /n 4 localhost >nul
echo Programming BL-Fuses...
avr32program -c USB:00A200006E88 -pavrdragon --part UC3B0256 writefuses -finternal@0x80000000 gp=0xfff5ffff
goto ende
:message
echo Call "Init-DVRPTR.bat <name of bootloader binary>"
:end
echo Done.
