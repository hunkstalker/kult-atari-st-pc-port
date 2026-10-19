@echo off
rem T-073c: run normal con traza Y+V (KULT_FRAME_GATE=0)
set KULT_FRAME_GATE=0
set KULT_MARKS=%~dp0kult-marcas.log
set KULT_VBL_TITLE=1
"%~dp0kult.exe"
