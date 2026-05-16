@echo off
setlocal enabledelayedexpansion

:: 设置两个文件夹路径
set "DirA=Cache"
set "DirB=Physics"

:: 第一步：收集 A 中所有文件的“无后缀名”（转成小写）到临时文件
set "ListFile=%temp%\~name_list.tmp"
if exist "%ListFile%" del "%ListFile%"

for /r "%DirA%" %%a in (*) do (
    echo %%~na>>"%ListFile%"
)

:: 第二步：遍历 B 中所有文件，如果无后缀名（转成小写）不在 ListFile 中就删除
for /r "%DirB%" %%f in (*) do (
    set "BaseName=%%~nf"
    findstr /i /x /c:"!BaseName!" "%ListFile%" >nul 2>nul
    if errorlevel 1 (
        echo 删除: "%%f"
        del "%%f"
    )
)

:: 第三步：删除临时文件
del "%ListFile%"

echo 清理完成.
pause