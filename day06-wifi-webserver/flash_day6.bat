@echo off
REM 一键烧录 Day6（WiFi + WebServer）到 ESP32-S3
REM 双击本文件即可，不依赖 VS Code 当前打开的是哪个工程

cd /d D:\100-days-embedded\day06-wifi-webserver

"C:\Users\Z1349\.workbuddy\binaries\python\envs\default\Scripts\platformio.exe" run -t upload

echo.
echo ===== 烧录结束，看上面的 SUCCESS 或 ERROR =====
pause
