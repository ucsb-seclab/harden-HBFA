@echo off
rem generate all CHM files at once

dir /s /b dxygn.bat

for /F %%a in ('dir /s /b dxygn.bat') do (
    cd %%~pa
    call dxygn.bat
    cd %%~p0
)
