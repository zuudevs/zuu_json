param(
    [Parameter(Mandatory)]
    [ValidateSet("configure", "build", "run", "clean")]
    [string] $opt,

    [ValidateSet("Debug", "Release")]
    [string] $mode = "Debug",

    [ValidateSet("zuu_json", "zuu_json_benchmark")]
    [string] $target = "zuu_json"
)

$ROOT = Split-Path $PSScriptRoot -Parent
$BUILD_DIR = Join-Path $ROOT "build"
$BIN_DIR = Join-Path $ROOT "bin"
$optional = ""

if ($target -eq "zuu_json_benchmark") {
    $optional = "-DENABLE_BENCHMARK=ON"
}

Set-Location $ROOT

switch ($opt) {
    "configure" {
        Write-Host "[*] Configuring CMake ($mode)..." -ForegroundColor Cyan
        & cmake -B $BUILD_DIR $optional "-DCMAKE_BUILD_TYPE=$mode"
    }

    "build" {
        Write-Host "[*] Building target $target..." -ForegroundColor Cyan
        & cmake --build $BUILD_DIR
    }

    "run" {
        Write-Host "[*] Running $target.exe..." -ForegroundColor Green
        & "$BIN_DIR\$target.exe"
    }
    
    "clean" {
        Write-Host "[*] Membersihkan file build dan bin..." -ForegroundColor Yellow
        if (Test-Path $BUILD_DIR) { Remove-Item -Recurse -Force $BUILD_DIR }
        if (Test-Path $BIN_DIR) { Remove-Item -Recurse -Force $BIN_DIR }
        Write-Host "[v] Clean selesai!" -ForegroundColor Green
    }
}