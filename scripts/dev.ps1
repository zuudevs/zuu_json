param(
    [Parameter(Mandatory)]
    [ValidateSet("configure", "build", "run")]
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
        & cmake -B $BUILD_DIR $optional "-DCMAKE_BUILD_TYPE=$mode"
    }

    "build" {
        & cmake --build $BUILD_DIR
    }

    "run" {
        & "$BIN_DIR\$target.exe"
    }
}