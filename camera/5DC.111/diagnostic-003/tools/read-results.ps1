# SPDX-License-Identifier: GPL-2.0-only
# Read-only viewer for the two diagnostic files; no CF writes.
[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$CardRoot)
$ErrorActionPreference='Stop'
$root=(Resolve-Path -LiteralPath $CardRoot).Path
foreach($name in @('RB3BOOT.TXT','RB3LATE.TXT')) {
    Write-Host "--- $name ---"
    $path=Join-Path $root $name
    if(Test-Path -LiteralPath $path -PathType Leaf) {
        Get-Content -LiteralPath $path -Raw
        Write-Host "SHA256:" (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash
    } else {
        Write-Host 'NAO ENCONTRADO'
    }
}
Write-Host 'A ausencia de RB3LATE pode indicar falha/criacao da tarefa; nao prova causa especifica.'
