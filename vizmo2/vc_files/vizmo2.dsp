# Microsoft Developer Studio Project File - Name="vizmo2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=vizmo2 - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "vizmo2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vizmo2.mak" CFG="vizmo2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vizmo2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "vizmo2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "vizmo2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(STLDIR)" /I "$(QTDIR)\include" /I "..\Plum" /I "..\mathtool" /I "..\modelgraph" /I ".." /I "..\src" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QT_DLL" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 qt-mt230nc.lib qtmain.lib opengl32.lib glu32.lib gli.lib Plum.lib modelgraph.lib mathtool.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"LIBCMT" /out:"..\vizmo2.exe" /libpath:"$(QTDIR)\lib" /libpath:"..\lib"

!ELSEIF  "$(CFG)" == "vizmo2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(STLDIR)" /I "$(QTDIR)\include" /I "..\Plum" /I "..\mathtool" /I "..\modelgraph" /I ".." /I "..\src" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "QT_DLL" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 qt-mt230nc.lib qtmain.lib opengl32.lib glu32.lib glid.lib Plumd.lib modelgraphd.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"LIBCMT" /out:"..\vizmo2_debug.exe" /pdbtype:sept /libpath:"$(QTDIR)\lib" /libpath:"..\lib"

!ENDIF 

# Begin Target

# Name "vizmo2 - Win32 Release"
# Name "vizmo2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\src\gui\animation_gui.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\BoundingBoxModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\BoundingBoxParser.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gui\gui.cpp
# End Source File
# Begin Source File

SOURCE=..\src\main.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gui\main_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gui\moc_animation_gui.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gui\moc_main_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gui\moc_scene_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\PathLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\PathModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\QueryLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\QueryModel.cpp
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\Robot.cpp
# End Source File
# Begin Source File

SOURCE=..\src\gui\scene_win.cpp
# End Source File
# Begin Source File

SOURCE=..\src\vizmo2.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\src\gui\animation_gui.h

!IF  "$(CFG)" == "vizmo2 - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\vizmo2\src\gui
InputPath=..\src\gui\animation_gui.h
InputName=animation_gui

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "vizmo2 - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\vizmo2\src\gui
InputPath=..\src\gui\animation_gui.h
InputName=animation_gui

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\BoundingBoxModel.h
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\BoundingBoxParser.h
# End Source File
# Begin Source File

SOURCE=..\src\gui\gui.h
# End Source File
# Begin Source File

SOURCE=..\src\gui\main_win.h

!IF  "$(CFG)" == "vizmo2 - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\vizmo2\src\gui
InputPath=..\src\gui\main_win.h
InputName=main_win

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "vizmo2 - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\vizmo2\src\gui
InputPath=..\src\gui\main_win.h
InputName=main_win

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\PathLoader.h
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\PathModel.h
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\QueryLoader.h
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\QueryModel.h
# End Source File
# Begin Source File

SOURCE=..\src\EnvObj\Robot.h
# End Source File
# Begin Source File

SOURCE=..\src\gui\scene_win.h

!IF  "$(CFG)" == "vizmo2 - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\vizmo2\src\gui
InputPath=..\src\gui\scene_win.h
InputName=scene_win

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "vizmo2 - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=\vizmo2\src\gui
InputPath=..\src\gui\scene_win.h
InputName=scene_win

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\src\vizmo2.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
