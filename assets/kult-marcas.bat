@echo off
rem T-073c: marcas B/S/F/M de la build estandar; Y/V solo en build de diagnostico
set KULT_MARKS=%~dp0kult-marcas.log
set KULT_VBL_TITLE=1
"%~dp0kult.exe"
