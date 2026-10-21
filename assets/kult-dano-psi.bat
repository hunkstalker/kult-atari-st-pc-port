@echo off
rem Captura de dano: PSI POWER. Traza R + Y/V + lecturas ALEAT.
set KULT_MARKS=%~dp0dano-psi-marcas.log
set KULT_RINGLOG=%~dp0dano-psi-ring.log
set KULT_RANDLOG=%~dp0dano-psi-rnd.log
set KULT_INPUTLOG=%~dp0dano-psi-input.log
set KULT_VBL_TITLE=1
"%~dp0kult-diag.exe"
