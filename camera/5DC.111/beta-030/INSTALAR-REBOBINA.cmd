@echo off
setlocal
echo REBOBINA 0.3 beta - atualizacao do cartao que JA inicia Soup/REBOBINA
echo ATENCAO: NAO instala em cartao virgem, NAO ativa bootflags.
echo Desligue a camera e faca backup das fotos antes.
set /p CARD=Digite a raiz do CompactFlash (exemplo D:\):
if "%CARD%"=="" (echo Cancelado.& pause & exit /b 1)
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0tools\install-rebobina.ps1" -CardRoot "%CARD%" -Apply
echo.
pause
