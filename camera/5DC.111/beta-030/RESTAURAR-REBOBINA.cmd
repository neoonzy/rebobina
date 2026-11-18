@echo off
setlocal
echo REBOBINA - restauracao do SE.BIN anterior
set /p CARD=Digite a raiz do CompactFlash (exemplo D:\):
set /p BACKUP=Digite a pasta BACKUP salva no computador:
if "%CARD%"=="" (pause & exit /b 1)
if "%BACKUP%"=="" (pause & exit /b 1)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\restore-rebobina.ps1" -CardRoot "%CARD%" -BackupDir "%BACKUP%" -Apply
pause
