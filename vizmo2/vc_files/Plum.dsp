# Microsoft Developer Studio Project File - Name="Plum" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=Plum - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Plum.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Plum.mak" CFG="Plum - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Plum - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "Plum - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Plum - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(STLDIR)" /I "..\mathtool" /I "..\modelgraph" /I "..\Plum" /I ".." /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Plum.lib"

!ELSEIF  "$(CFG)" == "Plum - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(STLDIR)" /I "..\mathtool" /I "..\modelgraph" /I "..\Plum" /I ".." /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\lib\Plumd.lib"

!ENDIF 

# Begin Target

# Name "Plum - Win32 Release"
# Name "Plum - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\Plum\EnvObj\CmdParser.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\EnvLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\EnvModel.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\MapObj\MapLoader.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\MapObj\MapModel.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\MovieBYULoader.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\MultiBodyInfo.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\MultiBodyModel.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\Plum.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\PlumState.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\PlumUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\PolyhedronModel.cpp
# End Source File
# Begin Source File

SOURCE=..\Plum\MapObj\SimpleCfg.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\Plum\EnvObj\CmdParser.h
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\EnvLoader.h
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\EnvModel.h
# End Source File
# Begin Source File

SOURCE=..\Plum\GLModel.h
# End Source File
# Begin Source File

SOURCE=..\Plum\Graph.h
# End Source File
# Begin Source File

SOURCE=..\Plum\ILoadable.h
# End Source File
# Begin Source File

SOURCE=..\Plum\ITransform.h
# End Source File
# Begin Source File

SOURCE=..\Plum\MapObj\MapLoader.h
# End Source File
# Begin Source File

SOURCE=..\Plum\MapObj\MapModel.h
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\MovieBYULoader.h
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\MultiBodyInfo.h
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\MultiBodyModel.h
# End Source File
# Begin Source File

SOURCE=..\Plum\Plum.h
# End Source File
# Begin Source File

SOURCE=..\Plum\PlumObject.h
# End Source File
# Begin Source File

SOURCE=..\Plum\PlumState.h
# End Source File
# Begin Source File

SOURCE=..\Plum\PlumUtil.h
# End Source File
# Begin Source File

SOURCE=..\Plum\EnvObj\PolyhedronModel.h
# End Source File
# Begin Source File

SOURCE=..\Plum\MapObj\SimpleCfg.h
# End Source File
# End Group
# End Target
# End Project
