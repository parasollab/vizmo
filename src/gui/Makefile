INCPATH = -I..
VIZMO_DIR = ../..

TARGET   = ../../lib/libvgui.a

OBJS = GUI.o MainWin.o SceneWin.o AnimationGUI.o \
          ItemSelectionGUI.o FileListDialog.o \
	  ObjProperty.o OBPRMGUI.o AddObjDialog.o\
	  VizmoEditor.o QueryGUI.o CameraPosDialog.o CustomizePathDialog.o TextGUI.o MainMenu.o FileOptions.o SceneOptions.o RobotOptions.o EnvironmentOptions.o RoadmapOptions.o PathOptions.o CaptureOptions.o HelpOptions.o MovieSaveDialog.o SizeSliderDialog.o 

SRCMOC = $(patsubst %.h, moc_%.cpp, $(shell grep -l Q_OBJECT *.h))
OBJMOC = $(patsubst %.h, moc_%.o, $(shell grep -l Q_OBJECT *.h))

OBJECTS = $(OBJS) $(OBJMOC)
include $(VIZMO_DIR)/Makefile.VizmoDefaults

CLEAN += $(SRCMOC) 

####### MOC

.SECONDARY: $(SRCMOC)

moc_%.cpp: %.h
	$(MOC) $< -o $@

