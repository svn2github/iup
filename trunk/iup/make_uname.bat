@echo off
REM This builds all the libraries for 1 uname

FOR %%u IN (src srccd srccontrols srcmatrixex srcgl srcglcontrols srcplot srcpplot srcmglplot srcscintilla srcim srcimglib srcole srctuio srcweb srcledc srcview) DO call make_uname_lib.bat %%u %1 %2 %3 %4 %5 %6 %7 %8 %9

REM call make_uname_lib.bat srclua5 %1 %2 %3 %4 %5 %6 %7 %8 %9
REM call make_uname_lib.bat srcconsole %1 %2 %3 %4 %5 %6 %7 %8 %9

set USE_LUA51=Yes
call make_uname_lib.bat srclua5 %1 %2 %3 %4 %5 %6 %7 %8 %9
call make_uname_lib.bat srcconsole %1 %2 %3 %4 %5 %6 %7 %8 %9
set USE_LUA51=

set USE_LUA52=Yes
call make_uname_lib.bat srclua5 %1 %2 %3 %4 %5 %6 %7 %8 %9
call make_uname_lib.bat srcconsole %1 %2 %3 %4 %5 %6 %7 %8 %9
set USE_LUA52=

set USE_LUA53=Yes
call make_uname_lib.bat srclua5 %1 %2 %3 %4 %5 %6 %7 %8 %9
call make_uname_lib.bat srcconsole %1 %2 %3 %4 %5 %6 %7 %8 %9
set USE_LUA53=

call make_uname_lib.bat html\examples\tests %1 %2 %3 %4 %5 %6 %7 %8 %9
cd ..\..
