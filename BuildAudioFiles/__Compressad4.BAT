start "NEXTSTEPRC" /wait OpenTXspeaker.exe
REM Normalise and cut silence
for /r %%a in ("*.wav") do CALL SOUNDS\tools\sox --norm=-1 %%a %%~na.aif silence 1 0.1 0.1% reverse
REM Cut silence
for /r %%a in ("*.aif") do CALL SOUNDS\tools\sox %%a %%~na.wav silence 1 0.1 0.1% reverse 
REM Convert to ADPCM 4 bits
for %%a in ("*.wav") do CALL SOUNDS\tools\AD4CONVERTER -E4 %%a
del *.aif /s
del *.wav /s
md VOICE
move /y *.AD4 VOICE
rem del *.AD4 /s
echo off
cls
echo ******************************************
echo ******************************************
echo               NEXSTEPRC
echo ******************************************
echo ******************************************
echo LES FICHIERS SONT DANS LE REPERTOIRE VOICE
echo ******************************************
echo ******************************************
pause
:end
