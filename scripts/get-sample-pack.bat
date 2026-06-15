@echo off

if not exist "..\samples" (
    mkdir "..\samples"
)

cd /d "..\samples"

echo Get sample pack from repository...
git clone https://github.com/zuudevs/zuu_json_sample.git

pause