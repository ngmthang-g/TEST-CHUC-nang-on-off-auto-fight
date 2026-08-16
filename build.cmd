@echo off
setlocal
cd /d "%~dp0"
where zig >nul 2>nul
if errorlevel 1 (echo KHONG TIM THAY ZIG & exit /b 1)
if not exist dist mkdir dist
del /q dist\* >nul 2>nul

echo [1/7] AutoFight test scope audit...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $files=(Get-ChildItem 'src' -Recurse -File | Where-Object {$_.Extension -in '.cpp','.h','.inl'} | Select-Object -ExpandProperty FullName); $s=($files|%%{Get-Content $_ -Raw -Encoding UTF8}) -join [Environment]::NewLine;" ^
  "$forbidden=@('CreateRemoteThread','WriteProcessMemory','remote_worker','UIButton','HandleClickEvent','InvokeRevive','DauThai','Đầu thai','ClickInternalConfirm','MessageBoxVisible','ClickNPC','RequestSellItem','RequestUsingSkill','SelectTarget','SendInput','mouse_event','SetCursorPos'); foreach($x in $forbidden){if($s -match [regex]::Escape($x)){throw ('Forbidden unrelated/visual-action token: '+$x)}};" ^
  "$bridge=(Get-Content 'src/bridge.cpp','src/bridge_runtime.inl','src/bridge_lua.inl' -Raw -Encoding UTF8) -join [Environment]::NewLine; foreach($x in @('LuaSystemAPI_GUI','AutoFight_Main','StartAutoFight','ProveUnityMainThread','UnitySynchronizationContext','MonoBehaviourExecutor','ExecuteUiObject','FindMethodExact','gchandle_new','value_box')){if($bridge -notmatch [regex]::Escape($x)){throw ('Missing AutoFight semantic/main-thread/donor token '+$x)}};" ^
  "$proto=Get-Content 'src/protocol.h' -Raw -Encoding UTF8; foreach($x in @('0x00020200','StartAutoFight = 1','StopAutoFight = 2','Probe = 3')){if($proto -notmatch [regex]::Escape($x)){throw ('Protocol missing '+$x)}};" ^
  "$ctl=Get-Content 'src/controller.cpp' -Raw -Encoding UTF8; foreach($x in @('v0.1.2','BẬT AUTO FIGHT','TẮT AUTO FIGHT','Command::StartAutoFight','Command::StopAutoFight')){if($ctl -notmatch [regex]::Escape($x)){throw ('Controller missing '+$x)}};" ^
  "Write-Host 'AUTOFIGHT TEST AUDIT PASS: direct AutoFight_Main.StartAutoFight service path; Unity-main-thread and frozen-donor guards; no mouse macro.'"
if errorlevel 1 exit /b 1

echo [2/7] Baseline route FSM regression self-test...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror src\route_logic_test.cpp -o dist\RouteLogicTest.exe
if errorlevel 1 exit /b 1
dist\RouteLogicTest.exe
if errorlevel 1 exit /b 1

echo [3/7] Build AutoFight bridge DLL...
rem IMPORTANT: do NOT combine -shared with -static here; that can emit an ar archive named .dll.
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -shared -s ^
  src\bridge.cpp -luser32 -lkernel32 -o dist\ThanLongAutoFightTestBridge.dll
if errorlevel 1 exit /b 1

echo [4/7] Verify bridge is a real PE DLL...
powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference='Stop'; $p='dist\ThanLongAutoFightTestBridge.dll'; $b=[IO.File]::ReadAllBytes($p); if($b.Length -lt 256){throw 'Bridge DLL too small'}; if($b[0] -ne 0x4D -or $b[1] -ne 0x5A){throw 'Bridge is not PE/MZ (wrong artifact type)'}; $pe=[BitConverter]::ToInt32($b,0x3C); if($pe -lt 0 -or $pe+24 -ge $b.Length){throw 'Invalid PE header offset'}; if($b[$pe] -ne 0x50 -or $b[$pe+1] -ne 0x45 -or $b[$pe+2] -ne 0 -or $b[$pe+3] -ne 0){throw 'Missing PE signature'}; $ch=[BitConverter]::ToUInt16($b,$pe+22); if(($ch -band 0x2000) -eq 0){throw 'PE does not have DLL characteristic'}; Write-Host ('BRIDGE PE DLL PASS characteristics=0x{0:X4}' -f $ch)"
if errorlevel 1 exit /b 1

echo [5/7] Build resources...
pushd resources
zig rc /c 65001 /fo ..\dist\app.res app.rc
popd
if errorlevel 1 exit /b 1

echo [6/7] Build controller EXE...
zig c++ -target x86_64-windows-gnu -O2 -std=c++17 -Wall -Wextra -Werror -municode -static -s ^
  src\controller.cpp dist\app.res -Wl,--subsystem,windows ^
  -lcomctl32 -luser32 -lkernel32 -lgdi32 ^
  -o dist\ThanLongAutoFightTest_v0.1.2.exe
if errorlevel 1 exit /b 1

echo [7/7] Done.
echo BUILD THANH CONG - AUTOFIGHT TEST v0.1.2
