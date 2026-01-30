::===================== File of the LUX Shader Project =====================::
::  - Initial D.  : 03.04.2025                                              ::
::  - Last Change : 29.01.2026                                              ::
::==========================================================================::
@echo off
setlocal EnableDelayedExpansion

rem Call this for the Process Name
call ShadersBuildDirectories.bat

rem Check all Process Names in the List
for %%P in (%PROCESS_LIST%) do (
    for /f "tokens=2 delims=," %%A in (
        'tasklist /FI "IMAGENAME eq %%P" /FO CSV /NH'
    ) do (
        if not "%%A"=="" (
            rem Get executable path from PID
            for /f "skip=1 tokens=*" %%X in (
                'wmic process where "ProcessId=%%~A" get ExecutablePath'
            ) do (
                set "EXEPATH=%%X"
                set "FOUND_PROCESS=%%P"
                goto :found
            )
        )
    )
)

rem Haven't found anything, doing nothing here
echo "No matching Process Name found, can't call +lux_vcshotreloads_pulse."
echo "Checked: %PROCESS_LIST%"
goto :eof

rem Found something, call it
:found
rem Trim WMIC trailing space
set "EXEPATH=%EXEPATH:~0,-1%"

echo "Found %FOUND_PROCESS%"
echo "Now requesting Materials that use these .vcs to be reloaded via lux_vcshotreloads_pulse."
echo "This will only work when vcshotreloads have been enabled using lux_vcshotreloads_toggle."
start "" "%EXEPATH%" -hijack +echo "Received Request for Materials & .vcs to be reloaded, by a .bat Script" +lux_vcshotreloads_pulse