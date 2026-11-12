# Copyright (c) Rebobina contributors. GPL-2.0-only.
# Inspect an upstream Soup Engine 5D release ZIP. READ ONLY: never touches the CF.
# Windows PowerShell 5.1+ / PowerShell 7+.
[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string] $ZipPath,
    # Snapshot of official GitHub API release asset "folders", recorded 2026-11-11.
    # Changes to upstream auto-generated releases REQUIRE a fresh review.
    [string] $ExpectedSha256 = '0f8cc374374d6692ec77836cd979ee75d2c77ae8a7a5bdd8da1cd5e6d0c9c993'
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Read-Prefix([System.IO.Compression.ZipArchiveEntry] $Entry, [int] $Count) {
    $buffer = New-Object byte[] $Count
    $stream = $Entry.Open()
    try {
        $offset = 0
        while ($offset -lt $Count) {
            $read = $stream.Read($buffer, $offset, $Count - $offset)
            if ($read -le 0) { throw "ZIP entry is truncated: $($Entry.FullName)" }
            $offset += $read
        }
        return ,$buffer
    }
    finally {
        $stream.Dispose()
    }
}

if ($ExpectedSha256 -notmatch '^[a-fA-F0-9]{64}$') {
    throw 'ExpectedSha256 must be a 64-character SHA-256 hex value.'
}
$file = (Resolve-Path -LiteralPath $ZipPath -ErrorAction Stop).ProviderPath
$info = Get-Item -LiteralPath $file -ErrorAction Stop
if ($info.PSIsContainer) { throw 'Expected a ZIP file, not a folder.' }
$hash = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
if ($hash -cne $ExpectedSha256.ToLowerInvariant()) {
    throw ("RELEASE ZIP SHA-256 MISMATCH. Expected: {0}; got: {1}. " +
           "Do not use this package until the new upstream release is independently reviewed.") -f $ExpectedSha256, $hash
}

Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = [System.IO.Compression.ZipFile]::OpenRead($file)
try {
    $autos = @($zip.Entries | Where-Object { $_.FullName -match '(^|/)card/AUTOEXEC\.BIN$' })
    $payloads = @($zip.Entries | Where-Object { $_.FullName -match '(^|/)card/SE\.BIN$' })
    $stocks = @($zip.Entries | Where-Object { $_.FullName -match '(^|/)card/ML/FILM/[^/]+\.fs$' })
    if ($autos.Count -ne 1 -or $payloads.Count -ne 1 -or $stocks.Count -lt 1) {
        throw ('Unexpected package layout: AUTOEXEC={0}, SE={1}, film profiles={2}.' -f
               $autos.Count, $payloads.Count, $stocks.Count)
    }
    $auto = $autos[0]
    $se = $payloads[0]
    if ($auto.Length -ne 221184) {
        throw "Unexpected legacy AUTOEXEC.BIN size: $($auto.Length) bytes."
    }
    if ($se.Length -lt 256 -or $se.Length -gt 131072) {
        throw "Unexpected SE.BIN size: $($se.Length) bytes."
    }
    $a = Read-Prefix $auto 12
    $b = Read-Prefix $se 4
    if ([System.Text.Encoding]::ASCII.GetString($a, 4, 8) -cne 'gaonisoy') {
        throw 'Legacy AUTOEXEC.BIN marker is missing.'
    }
    if ([System.Text.Encoding]::ASCII.GetString($b, 0, 4) -cne 'FILM') {
        throw 'SE.BIN does not have the expected FILM header.'
    }

    Write-Output 'PASS: Soup Engine release ZIP matches the pinned GitHub asset SHA-256.'
    Write-Output ('ZIP SHA-256: ' + $hash)
    Write-Output ('AUTOEXEC.BIN: {0} ({1} bytes, marker present)' -f $auto.FullName, $auto.Length)
    Write-Output ('SE.BIN: {0} ({1} bytes, FILM header present)' -f $se.FullName, $se.Length)
    Write-Output ('Film profiles found: {0}' -f $stocks.Count)
    Write-Output 'OFFLINE INSPECTION ONLY. Does NOT authenticate the underlying Magic Lantern binary.'
    Write-Output 'DO NOT COPY EXPERIMENTAL FILES TO A CF CARD OR CHANGE BOOT FLAGS.'
}
finally {
    $zip.Dispose()
}
