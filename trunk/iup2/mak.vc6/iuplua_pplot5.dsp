# Microsoft Developer Studio Project File - Name="iuplua_pplot5" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iuplua_pplot5 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iuplua_pplot5.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iuplua_pplot5.mak" CFG="iuplua_pplot5 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iuplua_pplot5 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iuplua_pplot5 - Win32 Debug"

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
# ADD BASE CPP /nologo /MT /I "..\src" /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\src" /I "..\include" /I "..\srclua5" /I "..\..\lua5.1\include" /I "..\..\cd\include" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "IUPLUA_USELOH" /D "_CRT_SECURE_NO_DEPRECATE" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /win32 
# ADD MTL /nologo /win32 
# ADD BASE RSC /l 1046 
# ADD RSC /l 1046 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"..\lib\iuplua_pplot51.lib" 
# ADD LIB32 /nologo /out:"..\lib\iuplua_pplot51.lib" 

!ENDIF

# Begin Target

# Name "iuplua_pplot5 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90"
# Begin Source File

SOURCE=..\srclua5\pplot.c
# End Source File
# Begin Source File

SOURCE=..\srclua5\pplotfuncs.c
# End Source File
# End Group
# Begin Group "Lua"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\srclua5\pplot.lua
# End Source File
# End Group
# Begin Group "Includes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\iuplua_pplot.h
# End Source File
# End Group
# End Target
# End Project

