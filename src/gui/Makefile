INCPATH = -I..
VIZMO_DIR = ../..

TARGET   = ../../lib/libvgui.a

OBJS = GUI.o MainWin.o SceneWin.o AnimationGUI.o \
          SnapshotGUI.o ItemSelectionGUI.o FileListDialog.o \
	  ObjProperty.o Roadmap.o OBPRMGUI.o AddObjDialog.o\
	  VizmoEditor.o QueryGUI.o CameraPosDialog.o TextGUI.o 

SRCMOC = $(patsubst %.h, moc_%.cpp, $(shell grep -l Q_OBJECT *.h))
OBJMOC = $(patsubst %.h, moc_%.o, $(shell grep -l Q_OBJECT *.h))

OBJECTS = $(OBJS) $(OBJMOC)
include $(VIZMO_DIR)/Makefile.VizmoDefaults

CLEAN += $(SRCMOC) 

####### MOC

.SECONDARY: $(SRCMOC)

moc_%.cpp: %.h
	$(MOC) $< -o $@

