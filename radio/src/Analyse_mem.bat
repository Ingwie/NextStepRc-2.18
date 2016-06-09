avr-size -A opentx.elf
avr-size -B opentx.elf
pause
avr-nm --size-sort opentx.elf>AnalyseMem.txt
call AnalyseMem.txt


