# SPDX-License-Identifier: GPL-2.0-only
<#
 Builds an OFF-CARD test folder and backups from an ALREADY WORKING 5DC CF.
 Never edits the CF, the camera boot flags, Canon firmware, or AUTOEXEC.BIN.
 Example: powershell -ExecutionPolicy Bypass -File .\tools\prepare-test.ps1 -CardRoot D:\
 Optional: -EnableLCD / -EnableLED only create opt-in marker files for manual use.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$CardRoot,
    [string]$Output = (Join-Path $env:USERPROFILE 'Downloads\REBOBINA-004-TESTE'),
    [switch]$EnableLCD,
    [switch]$EnableLED
)
$ErrorActionPreference = 'Stop'
$project = Split-Path -Parent $PSScriptRoot
$binFile = Join-Path $project 'dist\SE.BIN'
$manifestFile = Join-Path $project 'dist\manifest.json'
$card = (Resolve-Path -LiteralPath $CardRoot).Path
$autoexec = Join-Path $card 'AUTOEXEC.BIN'
$original = Join-Path $card 'SE.BIN'
if (Test-Path -LiteralPath $Output) { throw "Destino ja existe: $Output. Nao sobrescrever backups." }
foreach ($f in @($binFile, $manifestFile, $autoexec, $original)) {
    if (-not (Test-Path -LiteralPath $f -PathType Leaf)) { throw "Arquivo obrigatorio ausente: $f" }
}
if (-not (Test-Path -LiteralPath (Join-Path $card 'ML') -PathType Container)) {
    throw 'Pasta ML ausente. Nao parece ser o CF de teste da 5DC.'
}
$auto = [IO.File]::ReadAllBytes($autoexec)
if ($auto.Length -ne 221184 -or [Text.Encoding]::ASCII.GetString($auto,4,8) -cne 'gaonisoy') {
    throw 'AUTOEXEC.BIN nao tem a estrutura esperada do loader 5DC.'
}
function Get-U32([byte[]]$b,[int]$i) { [BitConverter]::ToUInt32($b,$i) }
function At([long]$addr) { [int]($addr - 0x00980000 + 0x1000) }
$init = Get-U32 $auto (At 0x00980774)
$gui = Get-U32 $auto (At 0x00989DB4)
if ($init -lt 0x00980820 -or $init -ge 0x0098090C) {
    throw 'Loader 5DC: entrada de init nao aponta para o stub Soup.'
}
if (([long]$gui -shr 24) -ne 0xEB) {
    throw 'Loader 5DC: hook de GUI inesperado.'
}
$rel = [long]($gui -band 0x00FFFFFF)
if (($rel -band 0x00800000) -ne 0) { $rel -= 0x01000000 }
$target = 0x00989DB4 + 8 + 4*$rel
if ($target -lt 0x00980820 -or $target -ge 0x0098090C) {
    throw 'Loader 5DC: hook GUI fora do stub Soup.'
}
$text = [Text.Encoding]::ASCII.GetString($auto, (At 0x00980820), 0xEC)
if (-not $text.Contains('A:/SE.BIN')) { throw 'Loader nao tem caminho esperado A:/SE.BIN.' }
$m = Get-Content -LiteralPath $manifestFile -Raw -Encoding UTF8 | ConvertFrom-Json
$bytes = [IO.File]::ReadAllBytes($binFile)
$actual = (Get-FileHash -LiteralPath $binFile -Algorithm SHA256).Hash.ToLowerInvariant()
if ($m.version -ne '0.0.4' -or $m.target -ne '5DC.111' -or
    $m.sha256 -ne $actual -or [int]$m.size -ne $bytes.Length -or
    [Text.Encoding]::ASCII.GetString($bytes,0,4) -cne 'FILM') {
    throw 'SE.BIN novo nao corresponde ao manifesto validado.'
}
# Preconditions validated; create output ON COMPUTER, never CF.
New-Item -ItemType Directory -Path $Output | Out-Null
$backup = Join-Path $Output 'BACKUP'
$payload = Join-Path $Output 'COPIAR-APENAS-SE.BIN'
New-Item -ItemType Directory -Path $backup,$payload | Out-Null
Copy-Item -LiteralPath $original -Destination (Join-Path $backup 'SE-ANTERIOR.BIN')
Copy-Item -LiteralPath $autoexec -Destination (Join-Path $backup 'AUTOEXEC-ANTERIOR.BIN')
Copy-Item -LiteralPath $binFile -Destination (Join-Path $payload 'SE.BIN')
Copy-Item -LiteralPath $manifestFile -Destination (Join-Path $Output 'manifest.json')
if ($EnableLCD -or $EnableLED) {
    New-Item -ItemType Directory -Path (Join-Path $payload 'ML') | Out-Null
    if ($EnableLCD) { [IO.File]::WriteAllBytes((Join-Path $payload 'ML\RB4LCD.ON'),[byte[]]@(49)) }
    if ($EnableLED) { [IO.File]::WriteAllBytes((Join-Path $payload 'ML\RB4LED.ON'),[byte[]]@(49)) }
}
$record = [ordered]@{
    target='Canon EOS 5D Classic 1.1.1'; firmware_verified_by_script=$false
    source_cf=$card; previous_se_sha256=(Get-FileHash -LiteralPath $original -Algorithm SHA256).Hash
    autoexec_sha256=(Get-FileHash -LiteralPath $autoexec -Algorithm SHA256).Hash
    new_se_sha256=$actual; enable_lcd=[bool]$EnableLCD; enable_led=[bool]$EnableLED
    note='No changes made to CF. Copy only staged SE.BIN (and optional ML marker files) manually.'
}
$record | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $Output 'BACKUP\identificacao.json') -Encoding UTF8
Write-Host "Preparado no computador: $Output" -ForegroundColor Green
Write-Host "BACKUP do arquivo SE.BIN anterior: $backup"
Write-Host "Copiar SOMENTE o SE.BIN dentro de: $payload"
Write-Host 'Nada foi gravado no CF. Nao reinstale Magic Lantern nem altere AUTOEXEC.BIN.'
