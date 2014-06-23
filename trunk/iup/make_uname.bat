@echo off
REM This builds all the libraries for 1 uname

if defined TECGRAF_INTERNAL goto tec_internal
goto tec_default

REM Includes Lua3
:tec_internal
call make_uname_lib.bat srclua3 %1 %2 %3 %4 %5 %6 %7 %8 %9
goto tec_default

:tec_default
FOR %%u IN (src srccd srccontrols srcmatrixex srcgl srcglcontrols srcpplot srcmglplot srcscintilla srcim srcimglib srcole srctuio srcweb srcledc srcview srclua5 srcconsole) DO call make_uname_lib.bat %%u %1 %2 %3 %4 %5 %6 %7 %8 %9
goto end

:end
