# SPDX-License-Identifier: GPL-2.0-only
<#
Rebobina 5DC.111 first physical-test *card staging* helper for Windows PowerShell 5.1.
This script NEVER writes a CompactFlash, boot flag, .FIR or Canon firmware.
Inputs: official Soup Canon-EOS-5D.zip (already SHA-256 checked), and the
rebobina-5dc-boot-probe-NAO-HOMOLOGADO ZIP from this branch's GitHub Actions.
Outputs: ~/Downloads/Rebobina-5DC-TESTE and ~/Downloads/Rebobina-5DC-ROLLBACK.
Both outputs must NOT already exist. The user manually copies files only
AFTER a 5DC.111 Magic Lantern boot baseline and accepts experimental risk.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$SoupZip,
    [Parameter(Mandatory=$true)][string]$RebobinaZip
)
$ErrorActionPreference = "Stop"
$expectedSoup = "0f8cc374374d6692ec77836cd979ee75d2c77ae8a7a5bdd8da1cd5e6d0c9c993"
$downloads = Join-Path $env:USERPROFILE "Downloads"
$dest = Join-Path $downloads "Rebobina-5DC-TESTE"
$rollback = Join-Path $downloads "Rebobina-5DC-ROLLBACK"
$tmp = Join-Path ([IO.Path]::GetTempPath()) ("rebobina-5dc-" + [guid]::NewGuid().ToString("N"))
function Require-File([string]$p) {
    $resolved = Resolve-Path -LiteralPath $p -ErrorAction Stop
    if (-not (Test-Path -LiteralPath $resolved.Path -PathType Leaf)) { throw "Arquivo inexistente: $p" }
    return $resolved.Path
}
function Read-U32([byte[]]$a,[int]$offset) {
    return [BitConverter]::ToUInt32($a,$offset)
}
function File-Off([long]$address) {
    return [int]($address - 0x00980000 + 0x1000)
}
$soupFile = Require-File $SoupZip
$rebobinaFile = Require-File $RebobinaZip
if ((Test-Path -LiteralPath $dest) -or (Test-Path -LiteralPath $rollback)) {
    throw "Destinos ja existem. Renomeie as pastas antigas antes de repetir o staging."
}
if ((Get-FileHash -LiteralPath $soupFile -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expectedSoup) {
    throw "Hash do ZIP Soup diferente do pacote conferido. Nao copie nada ao CF."
}
try {
    New-Item -ItemType Directory -Path $tmp | Out-Null
    $soupExtract = Join-Path $tmp "soup"
    $rbExtract = Join-Path $tmp "rebobina"
    Expand-Archive -LiteralPath $soupFile -DestinationPath $soupExtract -ErrorAction Stop
    Expand-Archive -LiteralPath $rebobinaFile -DestinationPath $rbExtract -ErrorAction Stop
    $card = Join-Path $soupExtract "Canon-EOS-5D\card"
    $baseLoader = Join-Path $card "AUTOEXEC.BIN"
    $origSe = Join-Path $card "SE.BIN"
    $probe = Join-Path $rbExtract "SE.BIN"
    $manifestFile = Join-Path $rbExtract "probe-manifest.json"
    foreach ($p in @($baseLoader,$origSe,$probe,$manifestFile)) {
        if (-not (Test-Path -LiteralPath $p -PathType Leaf)) { throw "Pacote incompleto: $p" }
    }
    if (-not (Test-Path -LiteralPath (Join-Path $card "ML\FILM") -PathType Container)) {
        throw "Pacote Soup sem ML/FILM."
    }
    $a = [IO.File]::ReadAllBytes($baseLoader)
    if ($a.Length -ne 221184 -or
        [Text.Encoding]::ASCII.GetString($a,4,8) -ne "gaonisoy") {
        throw "AUTOEXEC.BIN nao corresponde ao layout de referencia 5DC."
    }
    $stubLow = 0x00980820
    $stubHigh = 0x0098090C
    $init = Read-U32 $a (File-Off 0x00980774)
    if ($init -lt $stubLow -or $init -ge $stubHigh) {
        throw "Hook de init do Soup nao corresponde ao loader esperado."
    }
    $gui = Read-U32 $a (File-Off 0x00989DB4)
    if (([long]$gui -shr 24) -ne 0xEB) {
        throw "Hook GUI do Soup nao eh um branch BL."
    }
    $imm = [long]$gui -band 0xffffff
    if (($imm -band 0x800000) -ne 0) { $imm = $imm - 0x1000000 }
    $target = 0x00989DB4 + 8 + 4*$imm
    if ($target -lt $stubLow -or $target -ge $stubHigh) {
        throw "Hook GUI aponta fora do stub Soup."
    }
    $stubText = [Text.Encoding]::ASCII.GetString($a,(File-Off $stubLow),($stubHigh - $stubLow))
    if (-not $stubText.Contains("A:/SE.BIN")) {
        throw "Loader Soup nao inclui o caminho do payload."
    }
    $m = Get-Content -LiteralPath $manifestFile -Raw -Encoding UTF8 | ConvertFrom-Json
    $probeHash = (Get-FileHash -LiteralPath $probe -Algorithm SHA256).Hash.ToLowerInvariant()
    $probeBytes = [IO.File]::ReadAllBytes($probe)
    if ($m.target -ne "5DC.111" -or $m.hardware_verified -ne $false -or
        $m.sha256.ToLowerInvariant() -ne $probeHash -or
        [int]$m.size -ne $probeBytes.Length -or
        [Text.Encoding]::ASCII.GetString($probeBytes,0,4) -ne "FILM") {
        throw "Rebobina SE.BIN nao corresponde ao manifesto de teste."
    }
    # Only now create staging folders, in the user's Downloads.
    New-Item -ItemType Directory -Path $dest | Out-Null
    New-Item -ItemType Directory -Path $rollback | Out-Null
    Copy-Item -Path (Join-Path $card "*") -Destination $dest -Recurse -Force
    Copy-Item -LiteralPath $origSe -Destination (Join-Path $rollback "SOUP-SE-original.BIN")
    Copy-Item -LiteralPath $baseLoader -Destination (Join-Path $rollback "SOUP-AUTOEXEC-original.BIN")
    Copy-Item -LiteralPath $probe -Destination (Join-Path $dest "SE.BIN") -Force
    Copy-Item -LiteralPath $manifestFile -Destination (Join-Path $rollback "probe-manifest.json")
    Write-Host "STAGING PRONTO. Nada foi escrito no cartao CF." -ForegroundColor Green
    Write-Host "Arquivos de teste: $dest"
    Write-Host "Backup Soup: $rollback"
    Write-Host "SHA-256 Rebobina: $probeHash"
    Write-Host "ATENCAO: NAO copie ao CF sem primeiro inicializar 5DC.111 ML e aceitar riscos."
} finally {
    if (Test-Path -LiteralPath $tmp) {
        Remove-Item -LiteralPath $tmp -Recurse -Force -ErrorAction SilentlyContinue
    }
}
