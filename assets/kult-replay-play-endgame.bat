@echo off
rem Reproduce kult-replay-endgame.bin desde save-endgame sin restaurar save-t088.
if not exist "%~dp0kult-replay-endgame.bin" (
  echo No existe "%~dp0kult-replay-endgame.bin". Ejecuta antes kult-replay-record-endgame.bat.
  pause
  exit /b 1
)
set KULT_REPLAY_PLAY=%~dp0kult-replay-endgame.bin
if exist "%~dp0save-endgame\SCENA.BIN" copy /Y "%~dp0save-endgame\SCENA.BIN" "%~dp0data\SCENA.BIN" >nul
if exist "%~dp0save-endgame\CLEAR.BIN" copy /Y "%~dp0save-endgame\CLEAR.BIN" "%~dp0data\CLEAR.BIN" >nul
set KULT_FPSLOG=0
"%~dp0kult.exe"
