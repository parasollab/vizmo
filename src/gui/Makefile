INCPATH = -I..
VIZMO_DIR = ../..
include $(VIZMO_DIR)/Makefile.VizmoDefaults

ifeq ($(RPM), 0)
TARGET   = ../../lib/libvgui.a
else
TARGET   = ../../lib/libvgui.so
endif

####### Output directory

OBJECTS = GUI.o	MainWin.o SceneWin.o AnimationGUI.o \
          SnapshotGUI.o ItemSelectionGUI.o FileListDialog.o \
	  ObjProperty.o Roadmap.o OBPRMGUI.o AddObjDialog.o\
	  VizmoEditor.o QueryGUI.o TextGUI.o 

SRCMOC = $(patsubst %.h, moc_%.cpp, $(shell grep -l Q_OBJECT *.h))
OBJMOC = $(patsubst %.h, moc_%.o, $(shell grep -l Q_OBJECT *.h))

####### Build rules

ifeq ($(RPM), 0)
$(TARGET): $(OBJECTS) $(OBJMOC)
	$(AR) $(TARGET) ${OBJECTS} $(OBJMOC)
else
$(TARGET): $(OBJECTS) $(OBJMOC)
	$(CC) -shared $(OBJECTS) $(OBJMOC) -Wl,-soname=`basename $(TARGET)`.0 -o $(TARGET)
endif

default_target: $(TARGET)

CLEAN = $(OBJMOC) $(SRCMOC) $(OBJECTS) Dependencies *~ $(TARGET)

####### MOC

.SECONDARY: $(SRCMOC)

moc_%.cpp: %.h
	$(MOC) $< -o $@

-include Dependencies
