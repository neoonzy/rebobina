# SPDX-License-Identifier: GPL-2.0-only
# Guided update for an existing, verified Soup/REBOBINA 5DC card.
# Without -Apply the CF is read-only. No AUTOEXEC.BIN or bootflag changes.
[CmdletBinding()]
param(
 [Parameter(Mandatory=$true)][string]$CardRoot,
 [switch]$Apply,
 [string]$Output
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$card=(Resolve-Path -LiteralPath $CardRoot).Path
$preflight=Join-Path $PSScriptRoot 'prepare-test.ps1'
if(-not $Output) {
 $stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
 $nonce=([guid]::NewGuid().ToString('N')).Substring(0,8)
 $Output=Join-Path $env:USERPROFILE ("Downloads\REBOBINA-030-$stamp-$nonce")
}
& $preflight -CardRoot $card -Output $Output
$stage=Join-Path $Output 'COPIAR-PARA-CF'
$backup=Join-Path $Output 'BACKUP'
$srcBin=Join-Path $stage 'SE.BIN'
$srcMarker=Join-Path $stage 'ML\RB9BETA.ON'
$dstBin=Join-Path $card 'SE.BIN'
$dstMarker=Join-Path $card 'ML\RB9BETA.ON'
$old=Join-Path $backup 'SE-ANTERIOR.BIN'
foreach($file in @($srcBin,$srcMarker,$old)){
 if(-not (Test-Path -LiteralPath $file -PathType Leaf)){throw "Staging incompleto: $file"}
}
$hadMarker=Test-Path -LiteralPath $dstMarker -PathType Leaf
if($hadMarker) {
 Copy-Item -LiteralPath $dstMarker -Destination (Join-Path $backup 'RB9BETA-ANTERIOR.ON')
} else {
 'ABSENT' | Set-Content -LiteralPath (Join-Path $backup 'RB9BETA-NAO-EXISTIA.txt') -Encoding Ascii
}
$newHash=(Get-FileHash -LiteralPath $srcBin -Algorithm SHA256).Hash
Write-Host "Backup fora do CF: $backup" -ForegroundColor Green
Write-Host "SE.BIN novo SHA256: $newHash"
if(-not $Apply) {
 Write-Host 'Simulacao concluida. O CF nao foi alterado.'
 Write-Host 'Execute novamente com -Apply para instalar, gerando backup novo.'
 return
}
Write-Host 'ATENCAO: camera desligada, fotos em backup, cartao no leitor.' -ForegroundColor Yellow
Write-Host 'Isto nao habilita bootflags nem remove o Magic Lantern/Soup.'
$answer=Read-Host 'Digite exatamente INSTALAR para atualizar SE.BIN e RB9BETA.ON'
if($answer -cne 'INSTALAR'){Write-Host 'Cancelado sem gravar no CF.';return}
try {
 Copy-Item -LiteralPath $srcBin -Destination $dstBin -Force
 if((Get-FileHash -LiteralPath $dstBin -Algorithm SHA256).Hash -cne $newHash) {
  throw 'Integridade do SE.BIN no CF falhou.'
 }
 Copy-Item -LiteralPath $srcMarker -Destination $dstMarker -Force
 $marker=[IO.File]::ReadAllBytes($dstMarker)
 if($marker.Length -ne 1 -or $marker[0] -ne 49){throw 'Marcador no CF invalido.'}
 Write-Host 'Atualizacao REBOBINA 0.3 instalada e verificada.' -ForegroundColor Green
 Write-Host "Guarde para restauracao: $backup"
 Write-Host 'Ejete o CF com seguranca antes de colocar na camera.'
} catch {
 Write-Warning 'Erro ao instalar: tentando rollback do SE.BIN anterior.'
 try {
  Copy-Item -LiteralPath $old -Destination $dstBin -Force
  $expected=(Get-FileHash -LiteralPath $old -Algorithm SHA256).Hash
  if((Get-FileHash -LiteralPath $dstBin -Algorithm SHA256).Hash -cne $expected){
   throw 'Rollback SHA256 falhou.'
  }
  if($hadMarker) {
   Copy-Item -LiteralPath (Join-Path $backup 'RB9BETA-ANTERIOR.ON') -Destination $dstMarker -Force
  } elseif(Test-Path -LiteralPath $dstMarker){
   Remove-Item -LiteralPath $dstMarker -Force
  }
  Write-Warning 'SE.BIN anterior restaurado.'
 } catch {
  Write-Warning 'ROLLBACK INCOMPLETO. NAO LIGUE A CAMERA COM ESSE CARTAO. Verifique o BACKUP.'
 }
 throw
}
