@echo off
rem T-088: reproduce kult-replay.bin con las mismas respuestas de entrada.
if not exist "%~dp0kult-replay.bin" (
  echo No existe "%~dp0kult-replay.bin". Ejecuta antes kult-replay-record.bat.
  pause
  exit /b 1
)
set KULT_REPLAY_PLAY=%~dp0kult-replay.bin
if exist "%~dp0save-t088\SCENA.BIN" copy /Y "%~dp0save-t088\SCENA.BIN" "%~dp0data\SCENA.BIN" >nul
set KULT_FPSLOG=0
"%~dp0kult.exe"
