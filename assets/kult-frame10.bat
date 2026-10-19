@echo off
rem T-073c: prueba de ritmo, 10 VBL por banco (4 bancos del retrato = 40 VBL)
set KULT_FRAME_GATE=10
set KULT_MARKS=%~dp0kult-frame10.log
set KULT_VBL_TITLE=1
"%~dp0kult.exe"
