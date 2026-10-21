@echo off
rem Captura unica de todos los ataques (normal + PSI): R + Y/V + ALEAT + input.
set KULT_MARKS=%~dp0dano-todo-marcas.log
set KULT_RINGLOG=%~dp0dano-todo-ring.log
set KULT_RANDLOG=%~dp0dano-todo-rnd.log
set KULT_INPUTLOG=%~dp0dano-todo-input.log
set KULT_VBL_TITLE=1
"%~dp0kult-diag.exe"
