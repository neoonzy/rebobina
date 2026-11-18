# SPDX-License-Identifier: GPL-2.0-only
# Windows CI: install and rollback against a synthetic CF directory only.
# Does not talk to any real Canon camera or removable media.
$ErrorActionPreference='Stop'
$project=Split-Path -Parent $PSScriptRoot
$temp=Join-Path $env:TEMP ("rb030-smoke-"+[guid]::NewGuid().ToString('N'))
$card=Join-Path $temp 'fake-card'
$dist=Join-Path $project 'dist'
New-Item -ItemType Directory -Path (Join-Path $card 'ML'),$dist -Force | Out-Null
try {
    function At([long]$addr) {return [int]($addr - 0x00980000 + 0x1000)}
    $loader=New-Object byte[] 221184
    [Text.Encoding]::ASCII.GetBytes('gaonisoy').CopyTo($loader,4)
    [BitConverter]::GetBytes([uint32]0x00980850).CopyTo($loader,(At 0x00980774))
    $disp=[long]((0x00980850 - (0x00989DB4+8)) / 4)
    $branch=[uint32](([long]3942645760 -bor ($disp -band 0x00FFFFFF)))
    [BitConverter]::GetBytes($branch).CopyTo($loader,(At 0x00989DB4))
    [Text.Encoding]::ASCII.GetBytes('A:/SE.BIN').CopyTo($loader,((At 0x00980820)+64))
    [IO.File]::WriteAllBytes((Join-Path $card 'AUTOEXEC.BIN'),$loader)
    $old=New-Object byte[] 256
    [Text.Encoding]::ASCII.GetBytes('FILM').CopyTo($old,0)
    [IO.File]::WriteAllBytes((Join-Path $card 'SE.BIN'),$old)
    $new=New-Object byte[] 512
    [Text.Encoding]::ASCII.GetBytes('FILM').CopyTo($new,0)
    $new[256]=42
    $newFile=Join-Path $dist 'SE.BIN'
    [IO.File]::WriteAllBytes($newFile,$new)
    $newHash=(Get-FileHash -LiteralPath $newFile -Algorithm SHA256).Hash
    $manifest=[ordered]@{version='0.3.0-beta';target='5DC.111';size=512;sha256=$newHash.ToLowerInvariant()}
    $manifest | ConvertTo-Json | Set-Content (Join-Path $dist 'manifest.json') -Encoding UTF8
    $oldHash=(Get-FileHash -LiteralPath (Join-Path $card 'SE.BIN') -Algorithm SHA256).Hash
    & (Join-Path $project 'tools/install-rebobina.ps1') -CardRoot $card -Output (Join-Path $temp 'dry')
    if((Get-FileHash (Join-Path $card 'SE.BIN') -Algorithm SHA256).Hash -cne $oldHash) {
        throw 'Dry-run unexpectedly modified the card'
    }
    if(Test-Path (Join-Path $card 'ML\RB9BETA.ON')) {throw 'Dry-run wrote the beta marker'}
    function global:Read-Host {param([string]$Prompt) return 'INSTALAR'}
    & (Join-Path $project 'tools/install-rebobina.ps1') -CardRoot $card -Output (Join-Path $temp 'apply') -Apply
    if((Get-FileHash (Join-Path $card 'SE.BIN') -Algorithm SHA256).Hash -cne $newHash) {
        throw 'New payload not installed correctly'
    }
    if(-not (Test-Path (Join-Path $card 'ML\RB9BETA.ON'))) {throw 'Marker missing after install'}
    function global:Read-Host {param([string]$Prompt) return 'RESTAURAR'}
    & (Join-Path $project 'tools/restore-rebobina.ps1') -CardRoot $card -BackupDir (Join-Path $temp 'apply\BACKUP') -Apply
    if((Get-FileHash (Join-Path $card 'SE.BIN') -Algorithm SHA256).Hash -cne $oldHash) {
        throw 'Original SE.BIN not restored correctly'
    }
    if(Test-Path (Join-Path $card 'ML\RB9BETA.ON')) {throw 'New marker not removed on rollback'}
    # Reject invalid loader even when all other files are present.
    $loader[4]=0
    [IO.File]::WriteAllBytes((Join-Path $card 'AUTOEXEC.BIN'),$loader)
    $rejected=$false
    try {
      & (Join-Path $project 'tools/install-rebobina.ps1') -CardRoot $card -Output (Join-Path $temp 'bad')
    } catch { $rejected=$true }
    if(-not $rejected){throw 'Corrupted loader was accepted'}
    Write-Host 'PASS: install dry-run, apply, SHA256, rollback and invalid-loader rejection (fake CF only)'
}
finally {
    Remove-Item function:Read-Host -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath $temp -Recurse -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath (Join-Path $dist 'SE.BIN') -Force -ErrorAction SilentlyContinue
    Remove-Item -LiteralPath (Join-Path $dist 'manifest.json') -Force -ErrorAction SilentlyContinue
}
