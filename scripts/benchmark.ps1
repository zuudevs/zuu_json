param(
    [Parameter(Mandatory=$false)]
    [ValidateSet("fast", "full", "custom")]
    [string] $type = "fast",

    [Parameter(Mandatory=$false)]
    [string] $filter = "",

    [Parameter(Mandatory=$false)]
    [switch] $exportJson
)

$ROOT = Split-Path $PSScriptRoot -Parent
$BUILD_DIR = Join-Path $ROOT "build"
$BIN_DIR = Join-Path $ROOT "bin"
$EXE_PATH = Join-Path $BIN_DIR "zuu_json_benchmark.exe"

Set-Location $ROOT

Write-Host "[*] Mempersiapkan Build untuk Benchmark (Release Mode)..." -ForegroundColor Cyan

# 1. Selalu pastikan build dalam mode Release untuk Benchmark
& cmake -B $BUILD_DIR -DENABLE_BENCHMARK=ON -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { Write-Error "CMake Configure gagal!"; exit $LASTEXITCODE }

# 2. Build target benchmark
Write-Host "[*] Compiling zuu_json_benchmark..." -ForegroundColor Cyan
& cmake --build $BUILD_DIR --target zuu_json_benchmark --config Release
if ($LASTEXITCODE -ne 0) { Write-Error "Build gagal!"; exit $LASTEXITCODE }

# 3. Setup Google Benchmark Flags
$benchFlags = @()

if ($type -eq "fast") {
    # Fast: Hanya run file kecil/menengah dan pipeline utamanya
    Write-Host "[*] Mode: FAST BENCHMARK" -ForegroundColor Yellow
    $benchFlags += "--benchmark_filter=.*(Small|Medium).*FullPipeline.*"
}
elseif ($type -eq "custom") {
    # Custom: Gunakan filter regex dari user
    Write-Host "[*] Mode: CUSTOM BENCHMARK (Filter: $filter)" -ForegroundColor Yellow
    if ([string]::IsNullOrWhiteSpace($filter)) {
        Write-Warning "Kamu memilih 'custom' tapi tidak mengisi parameter -filter. Semua test akan dijalankan."
    } else {
        $benchFlags += "--benchmark_filter=$filter"
    }
}
else {
    # Full: Run semuanya
    Write-Host "[*] Mode: COMPREHENSIVE BENCHMARK" -ForegroundColor Yellow
}

if ($exportJson) {
    $outPath = Join-Path $ROOT "benchmark_result.json"
    $benchFlags += "--benchmark_format=json"
    $benchFlags += "--benchmark_out=$outPath"
    Write-Host "[*] Hasil akan diexport ke: benchmark_result.json" -ForegroundColor Green
}

# 4. Jalankan Benchmark
Write-Host "`n[*] Menjalankan Benchmark..." -ForegroundColor Cyan
& $EXE_PATH $benchFlags