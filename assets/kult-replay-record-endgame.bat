@echo off
rem Graba un replay desde save-endgame sin restaurar save-t088.
set KULT_REPLAY_REC=%~dp0kult-replay-endgame.bin
if exist "%~dp0save-endgame\SCENA.BIN" copy /Y "%~dp0save-endgame\SCENA.BIN" "%~dp0data\SCENA.BIN" >nul
if exist "%~dp0save-endgame\CLEAR.BIN" copy /Y "%~dp0save-endgame\CLEAR.BIN" "%~dp0data\CLEAR.BIN" >nul
set KULT_FPSLOG=0
"%~dp0kult.exe"
