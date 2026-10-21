@echo off
rem T-088: graba las respuestas de entrada (cursor, clics y teclas) en kult-replay.bin.
rem Ejecuta la partida hasta el crash; el fichero se vuelca en cada sondeo.
set KULT_REPLAY_REC=%~dp0kult-replay.bin
if exist "%~dp0save-t088\SCENA.BIN" copy /Y "%~dp0save-t088\SCENA.BIN" "%~dp0data\SCENA.BIN" >nul
set KULT_FPSLOG=0
"%~dp0kult.exe"
