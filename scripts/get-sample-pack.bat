@echo off

set "SCRIPT_DIR=%~dp0"

if not exist "%SCRIPT_DIR%..\samples" (
    mkdir "%SCRIPT_DIR%..\samples"
)

cd /d "%SCRIPT_DIR%..\samples"

echo Get sample pack from repository...
git clone https://github.com/zuudevs/zuu_json_sample.git

echo Moving sample pack content to sample root...
move "zuu_json_sample\*" .

echo Removing cloned repository...
rmdir /s /q "zuu_json_sample"