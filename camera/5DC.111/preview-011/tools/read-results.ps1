# SPDX-License-Identifier: GPL-2.0-only
# Read only; never modifies CompactFlash. PowerShell 5.1+.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$CardRoot)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $CardRoot).Path
foreach($name in @('RB6BOOT.TXT','RB6TEST.TXT','RB6EVENT.TXT','RB6FINAL.TXT')) {
 Write-Host "--- $name ---"
 $file=Join-Path $root $name
 if(Test-Path -LiteralPath $file -PathType Leaf) {
   Get-Content -LiteralPath $file -Raw
   Write-Host "SHA256:" (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash
 }else{
   Write-Host 'NAO ENCONTRADO'
 }
}
Write-Host 'Se somente RB6BOOT existir, a tarefa pode ter falhado ou ainda estar em execucao.'
Write-Host 'UI_DRAW_CALLS indica apenas chamadas de desenho, nao texto visivel no LCD.'
