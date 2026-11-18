# SPDX-License-Identifier: GPL-2.0-only
# Restore the previous SE.BIN and previous RB9 marker only.
[CmdletBinding()]
param(
 [Parameter(Mandatory=$true)][string]$CardRoot,
 [Parameter(Mandatory=$true)][string]$BackupDir,
 [switch]$Apply
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$card=(Resolve-Path -LiteralPath $CardRoot).Path
$backup=(Resolve-Path -LiteralPath $BackupDir).Path
$old=Join-Path $backup 'SE-ANTERIOR.BIN'
$meta=Join-Path $backup 'identificacao.json'
foreach($file in @($old,$meta,(Join-Path $card 'AUTOEXEC.BIN'),(Join-Path $card 'SE.BIN'))){
 if(-not (Test-Path -LiteralPath $file -PathType Leaf)){throw "Arquivo ausente: $file"}
}
if(-not (Test-Path (Join-Path $card 'ML') -PathType Container)){throw 'ML ausente no CF.'}
$manifest=Get-Content -LiteralPath $meta -Raw -Encoding UTF8 | ConvertFrom-Json
$hash=(Get-FileHash -LiteralPath $old -Algorithm SHA256).Hash
if($hash -cne $manifest.old_se_sha256){throw 'Backup nao confere com identificacao.json.'}
$prevMarker=Join-Path $backup 'RB9BETA-ANTERIOR.ON'
$absent=Join-Path $backup 'RB9BETA-NAO-EXISTIA.txt'
if(-not (Test-Path -LiteralPath $prevMarker) -and -not (Test-Path -LiteralPath $absent)){
 throw 'Backup nao registra o estado anterior do marcador.'
}
Write-Host "SE.BIN de backup SHA256: $hash"
if(-not $Apply){Write-Host 'Simulacao concluida. CF intocado.';return}
Write-Host 'Camera DESLIGADA, CF no leitor. AUTOEXEC.BIN nao sera modificado.' -ForegroundColor Yellow
$answer=Read-Host 'Digite exatamente RESTAURAR'
if($answer -cne 'RESTAURAR'){Write-Host 'Cancelado.';return}
$dst=Join-Path $card 'SE.BIN'
Copy-Item -LiteralPath $old -Destination $dst -Force
if((Get-FileHash -LiteralPath $dst -Algorithm SHA256).Hash -cne $hash){
 throw 'Falha na integridade do SE.BIN restaurado: NAO USE O CF.'
}
$marker=Join-Path $card 'ML\RB9BETA.ON'
if(Test-Path -LiteralPath $prevMarker -PathType Leaf){
 Copy-Item -LiteralPath $prevMarker -Destination $marker -Force
} elseif(Test-Path -LiteralPath $marker){
 Remove-Item -LiteralPath $marker -Force
}
Write-Host 'Restauracao verificada. O AUTOEXEC.BIN nao foi alterado.' -ForegroundColor Green
