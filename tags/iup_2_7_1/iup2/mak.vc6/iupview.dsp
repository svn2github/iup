# Microsoft Developer Studio Project File - Name="iupview" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=iupview - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iupview.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iupview.mak" CFG="iupview - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iupview - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iupview - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\bin"
# PROP BASE Intermediate_Dir "..\obj\$(ProjectName)"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\$(ProjectName)"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /I "..\include" /I "..\src" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "USE_IM" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD CPP /nologo /MT /I "..\include" /I "..\src" /Z7 /W4 /Od /D "WIN32" /D "_DEBUG" /D "USE_IM" /D "_CRT_SECURE_NO_DEPRECATE" /D "_MBCS" PRECOMP_VC7_TOBEREMOVED /GZ /c /GX 
# ADD BASE MTL /nologo /tlb"..\bin\debug\iupview.tlb" /win32 
# ADD MTL /nologo /tlb"..\bin\debug\iupview.tlb" /win32 
# ADD BASE RSC /l 1046 /d "_DEBUG" 
# ADD RSC /l 1046 /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo 
# ADD BSC32 /nologo 
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ole32.lib iup.lib iupcontrols.lib cd.lib freetype6.lib iupcd.lib iupgl.lib opengl32.lib glu32.lib iupimglib.lib im.lib iupim.lib /nologo /incremental:no /libpath:"..\..\im\lib" /libpath:"..\..\cd\lib" /libpath:"..\lib" /debug /pdb:"..\bin\iupview.pdb" /pdbtype:sept /subsystem:windows /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib ole32.lib iup.lib iupcontrols.lib cd.lib freetype6.lib iupcd.lib iupgl.lib opengl32.lib glu32.lib iupimglib.lib im.lib iupim.lib /nologo /incremental:no /libpath:"..\..\im\lib" /libpath:"..\..\cd\lib" /libpath:"..\lib" /debug /pdb:"..\bin\iupview.pdb" /pdbtype:sept /subsystem:windows /MACHINE:I386

!ENDIF

# Begin Target

# Name "iupview - Win32 Debug"
# Begin Source File

SOURCE=..\etc\iup.rc
# End Source File
# Begin Source File

SOURCE=..\srcview\iupview.c
# End Source File
# End Target
# End Project

