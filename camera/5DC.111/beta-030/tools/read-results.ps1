# SPDX-License-Identifier: GPL-2.0-only
# Read only; never modifies CompactFlash. PowerShell 5.1+.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$CardRoot)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $CardRoot).Path
foreach($name in @('RB9BOOT.TXT','RB9TEST.TXT','RB9SESSION.TXT')) {
 Write-Host "--- $name ---"
 $file=Join-Path $root $name
 if(Test-Path -LiteralPath $file -PathType Leaf) {
   Get-Content -LiteralPath $file -Raw
   Write-Host "SHA256:" (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash
 }else{
   Write-Host 'NAO ENCONTRADO'
 }
}
Write-Host 'RB9SESSION e gerado apenas ao escolher EXPORTAR RELATORIO no menu.'
Write-Host 'Leitura apenas: o script nao escreve no CF.'
