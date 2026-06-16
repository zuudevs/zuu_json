$ROOT = Split-Path $PSScriptRoot -Parent
$SAMPLES_DIR = Join-Path $ROOT "samples"

Write-Host "[*] Mengecek folder samples..." -ForegroundColor Cyan

if (-Not (Test-Path -Path $SAMPLES_DIR)) {
    Write-Host "[*] Membuat direktori samples..." -ForegroundColor Yellow
    New-Item -ItemType Directory -Force -Path $SAMPLES_DIR | Out-Null
}

Set-Location $SAMPLES_DIR

if (Test-Path (Join-Path $SAMPLES_DIR ".git")) {
    Write-Host "[*] Sample repository sudah ada. Mengambil update terbaru (git pull)..." -ForegroundColor Green
    & git pull origin main
} else {
    Write-Host "[*] Mengunduh (clone) sample pack dari repositori..." -ForegroundColor Yellow
    & git clone https://github.com/zuudevs/zuu_json_sample.git zuu_json_sample_temp
    
    Write-Host "[*] Memindahkan file ke root folder samples..." -ForegroundColor Cyan
    Get-ChildItem -Path "zuu_json_sample_temp" -Force | Move-Item -Destination . -Force
    
    Remove-Item -Path "zuu_json_sample_temp" -Force
}

Write-Host "[v] Selesai! Samples siap digunakan." -ForegroundColor Green