# SPDX-License-Identifier: GPL-2.0-only
<#
PowerShell 5.1 staging helper; ALWAYS writes on the computer, never to the CF.
Use -EnableUI to include ML/RB5UI.ON in the staged folder: this opts in to
an experimental Magic Lantern bitmap drawing call (camera may freeze).
Requires CF previously booting Soup Engine 5DC.111, a backup and battery charged.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$CardRoot,
    [string]$Output = (Join-Path $env:USERPROFILE 'Downloads\REBOBINA-010-TESTE'),
    [switch]$EnableUI
)
$ErrorActionPreference='Stop'
$project=Split-Path -Parent $PSScriptRoot
$binFile=Join-Path $project 'dist\SE.BIN'
$manifestFile=Join-Path $project 'dist\manifest.json'
$card=(Resolve-Path -LiteralPath $CardRoot).Path
$autoexec=Join-Path $card 'AUTOEXEC.BIN'
$oldSE=Join-Path $card 'SE.BIN'
if(Test-Path -LiteralPath $Output) { throw "Destino ja existe: $Output. Preserve os backups anteriores." }
foreach($file in @($binFile,$manifestFile,$autoexec,$oldSE)) {
 if(-not (Test-Path -LiteralPath $file -PathType Leaf)) { throw "Arquivo ausente: $file" }
}
if(-not (Test-Path -LiteralPath (Join-Path $card 'ML') -PathType Container)) {throw 'Pasta ML ausente no CF.'}
if((-not $EnableUI) -and (Test-Path -LiteralPath (Join-Path $card 'ML\RB5UI.ON') -PathType Leaf)) {
 throw 'O CF ja tem ML/RB5UI.ON. Apague manualmente este marcador antes de usar modo HEADLESS.'
}
$auto=[IO.File]::ReadAllBytes($autoexec)
if($auto.Length -ne 221184 -or [Text.Encoding]::ASCII.GetString($auto,4,8) -cne 'gaonisoy') {
 throw 'AUTOEXEC.BIN nao tem a estrutura de referencia do loader Soup 5DC.'
}
function At([long]$address) { return [int]($address - 0x00980000 + 0x1000) }
$init=[BitConverter]::ToUInt32($auto,(At 0x00980774))
$gui=[BitConverter]::ToUInt32($auto,(At 0x00989DB4))
if($init -lt 0x00980820 -or $init -ge 0x0098090C) { throw 'Hook de boot nao aponta para o stub Soup.' }
if(([long]$gui -shr 24) -ne 0xEB) {throw 'Hook GUI nao e BL.'}
$rel=[long]($gui -band 0xFFFFFF)
if(($rel -band 0x800000) -ne 0) {$rel -= 0x1000000}
$target=0x00989DB4+8+4*$rel
if($target -lt 0x00980820 -or $target -ge 0x0098090C) {throw 'Hook GUI aponta fora do stub Soup.'}
$text=[Text.Encoding]::ASCII.GetString($auto,(At 0x00980820),0xEC)
if(-not $text.Contains('A:/SE.BIN')) { throw 'Stub nao referencia A:/SE.BIN.' }
$m=Get-Content -LiteralPath $manifestFile -Raw -Encoding UTF8 | ConvertFrom-Json
$new=[IO.File]::ReadAllBytes($binFile)
$sha=(Get-FileHash -LiteralPath $binFile -Algorithm SHA256).Hash.ToLowerInvariant()
if($m.version -ne '0.1.0-dev' -or $m.target -ne '5DC.111' -or
   $m.sha256.ToLowerInvariant() -ne $sha -or [int]$m.size -ne $new.Length -or
   $new.Length -lt 256 -or $new.Length -gt 131072 -or
   [Text.Encoding]::ASCII.GetString($new,0,4) -cne 'FILM') {
 throw 'Payload REBOBINA e manifesto nao conferem.'
}
# All checks passed. Create staging BACK ON PC, not the CF.
$stage=Join-Path $Output 'COPIAR-PARA-CF'
$backup=Join-Path $Output 'BACKUP'
New-Item -ItemType Directory -Path $stage,$backup | Out-Null
Copy-Item -LiteralPath $oldSE -Destination (Join-Path $backup 'SE-ANTERIOR.BIN')
Copy-Item -LiteralPath $autoexec -Destination (Join-Path $backup 'AUTOEXEC-ANTERIOR.BIN')
Copy-Item -LiteralPath $binFile -Destination (Join-Path $stage 'SE.BIN')
Copy-Item -LiteralPath $manifestFile -Destination (Join-Path $Output 'manifest.json')
if($EnableUI) {
 $ml=Join-Path $stage 'ML'
 New-Item -ItemType Directory -Path $ml | Out-Null
 [IO.File]::WriteAllBytes((Join-Path $ml 'RB5UI.ON'), [byte[]]@(49))
}
$state=[ordered]@{
 target='5DC.111';mode= $(if($EnableUI){'FULL_UI_EXPERIMENTAL'}else{'HEADLESS_DIAGNOSTIC'})
 old_se_sha256=(Get-FileHash -LiteralPath $oldSE -Algorithm SHA256).Hash
 autoexec_sha256=(Get-FileHash -LiteralPath $autoexec -Algorithm SHA256).Hash
 new_se_sha256=$sha
 note='Only local staging files were created, never the CF.'
}
$state | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $backup 'identificacao.json') -Encoding UTF8
Write-Host "STAGING PRONTO: $stage" -ForegroundColor Green
Write-Host "Backup do SE.BIN anterior: $backup"
Write-Host "Modo: $($state.mode)"
Write-Host 'Copie manualmente somente o conteudo de COPIAR-PARA-CF para o CF com a camera desligada.'
Write-Host 'Nunca substitua AUTOEXEC.BIN, nunca execute instaladores, nao modifique fotos.'
