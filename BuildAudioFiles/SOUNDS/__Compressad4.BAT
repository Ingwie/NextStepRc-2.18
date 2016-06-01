for /r %%a in ("*.wav") do CALL tools\sox --norm %%a "%%~na.aif"
for /r %%a in ("*.aif") do CALL tools\sox %%a   "%%~na.wav"
for /r %%a in ("*.aif") do CALL tools\sox %%a   "%%~na.wav"
for /r %%a in ("*.aif") do CALL tools\sox %%a   "%%~na.wav"
for /r %%a in ("*.wav") do CALL tools\AD4CONVERTER -E4 %%a
del *.wav /s
del *.aif /s

pause
:end
