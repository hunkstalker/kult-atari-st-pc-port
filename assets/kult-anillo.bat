@echo off
rem Traza ligera del anillo: solo lineas R, sin el log masivo F/Y/V.
set KULT_RINGLOG=%~dp0kult-anillo.log
set KULT_VBL_TITLE=1
"%~dp0kult.exe"
