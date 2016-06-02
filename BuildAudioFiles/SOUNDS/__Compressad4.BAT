REM Normalise and cut silence
for /r %%a in ("*.wav") do CALL tools\sox --norm=-1 %%a %%~na.aif silence 1 0.1 0.1% reverse
REM Cut silence
for /r %%a in ("*.aif") do CALL tools\sox %%a %%~na.wav silence 1 0.1 0.1% reverse 
REM Convert to ADPCM 4 bits
for %%a in ("*.wav") do CALL tools\AD4CONVERTER -E4 %%a
del *.aif /s
rem del *.wav /s
rem del *.AD4 /s
pause
:end
