# Microsoft Developer Studio Project File - Name="iupluacontrols3" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iupluacontrols3 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iupluacontrols3.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iupluacontrols3.mak" CFG="iupluacontrols3 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iupluacontrols3 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iupluacontrols3 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\lib"
# PROP BASE Intermediate_Dir "..\obj\$(ProjectName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\lib"
# PROP Intermediate_Dir "..\obj\$(ProjectName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\include" /I "..\..\lua\include" /I "..\..\cd\include" /I "..\srclua3" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\include" /I "..\..\lua\include" /I "..\..\cd\include" /I "..\srclua3" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1046 
# ADD RSC /l 1046 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\iupluacontrols3.lib" 
# ADD LIB32 /nologo /out:"..\lib\iupluacontrols3.lib" 

!ENDIF

# Begin Target

# Name "iupluacontrols3 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\srclua3\luacb.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luacbox.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luacells.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luacolorbar.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luacontrols.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luadial.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luagauge.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luagc.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luagetparam.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luamask.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luamatrix.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luasbox.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luaspin.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luatabs.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luatree.c
# End Source File
# Begin Source File

SOURCE=..\srclua3\luaval.c
# End Source File
# End Group
# Begin Group "Lua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua3\luacb.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luacells.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luacolorbar.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luadial.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luagauge.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luamatrix.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luasbox.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luaspin.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luatabs.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luatree.lua
# End Source File
# Begin Source File

SOURCE=..\srclua3\luaval.lua
# End Source File
# End Group
# Begin Group "Includes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua3\il_controls.h
# End Source File
# Begin Source File

SOURCE=..\include\iupluacontrols.h
# End Source File
# End Group
# End Target
# End Project

