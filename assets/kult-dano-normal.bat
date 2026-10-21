@echo off
rem Captura de dano: ataque normal. Traza R + Y/V + lecturas ALEAT.
set KULT_MARKS=%~dp0dano-normal-marcas.log
set KULT_RINGLOG=%~dp0dano-normal-ring.log
set KULT_RANDLOG=%~dp0dano-normal-rnd.log
set KULT_INPUTLOG=%~dp0dano-normal-input.log
set KULT_VBL_TITLE=1
"%~dp0kult-diag.exe"
