@echo off
rem Replay generico (T-090): graba/reproduce sin depender de save-t088 ni endgame.
rem Uso:
rem   kult-replay-generic.bat record <carpeta-save> [replay.bin]
rem   kult-replay-generic.bat play   <carpeta-save> [replay.bin]
rem
rem <carpeta-save> puede ser cualquier carpeta con SCENA.BIN y/o CLEAR.BIN.
rem Si se omite replay.bin, usa kult-replay.bin junto a este .bat.
rem Copia el .bat junto a kult.exe (y data\) para usarlo desde el paquete.
setlocal
set "MODE=%~1"
set "SAVEDIR=%~2"
set "REPLAY=%~3"
if "%MODE%"=="" goto usage
if "%SAVEDIR%"=="" goto usage
if "%REPLAY%"=="" set "REPLAY=%~dp0kult-replay.bin"

set "KULT_REPLAY_REC="
set "KULT_REPLAY_PLAY="
if /I "%MODE%"=="record" set "KULT_REPLAY_REC=%REPLAY%"
if /I "%MODE%"=="play"   set "KULT_REPLAY_PLAY=%REPLAY%"
if not defined KULT_REPLAY_REC if not defined KULT_REPLAY_PLAY goto usage

if exist "%SAVEDIR%\SCENA.BIN" copy /Y "%SAVEDIR%\SCENA.BIN" "%~dp0data\SCENA.BIN" >nul
if exist "%SAVEDIR%\CLEAR.BIN" copy /Y "%SAVEDIR%\CLEAR.BIN" "%~dp0data\CLEAR.BIN" >nul
set "KULT_FPSLOG=0"
"%~dp0kult.exe"
exit /b %errorlevel%

:usage
echo Uso: %~nx0 record^|play ^<carpeta-save^> [replay.bin]
echo   record: KULT_REPLAY_REC=replay.bin
echo   play:   KULT_REPLAY_PLAY=replay.bin
echo   Copia SCENA.BIN/CLEAR.BIN de ^<carpeta-save^> a data\ si existen.
exit /b 2
