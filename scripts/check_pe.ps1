$path = "vendor/filament/out/release/filament/bin/matc.exe"
$bytes = [IO.File]::ReadAllBytes($path)

$sig = [BitConverter]::ToString($bytes, 0, 4)
Write-Host "Signature: $sig"

if ($sig -eq "4D-5A-90-00" -or $sig.StartsWith("4D-5A")) {
    Write-Host "Format: PE (Windows)"
    $peOffset = [BitConverter]::ToInt32($bytes, 0x3C)
    $machine = [BitConverter]::ToUInt16($bytes, $peOffset + 4)
    Write-Host "Machine: 0x$($machine.ToString('X'))"
} elseif ($sig.StartsWith("7F-45-4C-46")) {
    Write-Host "Format: ELF (Linux)"
} else {
    Write-Host "Format: Unknown"
}
