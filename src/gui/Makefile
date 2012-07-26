INCPATH = -I..
VIZMO_DIR = ../..
include $(VIZMO_DIR)/M_incl

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

SRCMOC = moc_MainWin.cpp moc_SceneWin.cpp moc_AnimationGUI.cpp moc_SnapshotGUI.cpp \
         moc_ItemSelectionGUI.cpp moc_Roadmap.cpp moc_TextGUI.cpp moc_FileListDialog.cpp moc_ObjProperty.cpp \
	 moc_OBPRMGUI.cpp moc_AddObjDialog.cpp moc_VizmoEditor.cpp moc_AttributeSelectionGUI.cpp moc_QueryGUI.cpp
OBJMOC = moc_MainWin.o moc_SceneWin.o moc_AnimationGUI.o moc_SnapshotGUI.o moc_ItemSelectionGUI.o \
         moc_Roadmap.o moc_TextGUI.o moc_FileListDialog.o moc_ObjProperty.o moc_OBPRMGUI.o \
	 moc_AddObjDialog.o moc_VizmoEditor.o moc_QueryGUI.o


####### Build rules

ifeq ($(RPM), 0)
$(TARGET): $(OBJECTS) $(OBJMOC)
	ar ruv $(TARGET) ${OBJECTS} $(OBJMOC)
#	libtool -static -o $(TARGET) ${OBJECTS} $(OBJMOC)
else
$(TARGET): $(OBJECTS) $(OBJMOC)
	$(CC) -shared $(OBJECTS) $(OBJMOC) -Wl,-soname=`basename $(TARGET)`.0 -o $(TARGET)
endif

default_target: $(TARGET)

CLEAN = $(OBJMOC) $(SRCMOC) $(OBJECTS) Dependencies *~ core *.core $(TARGET)

FORCE:

####### MOC

moc_MainWin.cpp: MainWin.h
	$(MOC) MainWin.h -o moc_MainWin.cpp

moc_SceneWin.cpp: SceneWin.h
	$(MOC) SceneWin.h -o moc_SceneWin.cpp

moc_AnimationGUI.cpp: AnimationGUI.h
	$(MOC) AnimationGUI.h -o moc_AnimationGUI.cpp

moc_ItemSelectionGUI.cpp: ItemSelectionGUI.h
	$(MOC) ItemSelectionGUI.h -o moc_ItemSelectionGUI.cpp

moc_Roadmap.cpp: Roadmap.h
	$(MOC) Roadmap.h -o moc_Roadmap.cpp

moc_TextGUI.cpp: TextGUI.h
	$(MOC) TextGUI.h -o moc_TextGUI.cpp 

moc_SnapshotGUI.cpp: SnapshotGUI.h
	$(MOC) SnapshotGUI.h -o moc_SnapshotGUI.cpp

moc_FileListDialog.cpp: FileListDialog.h
	$(MOC) FileListDialog.h -o moc_FileListDialog.cpp

moc_ObjProperty.cpp: ObjProperty.h
	$(MOC) ObjProperty.h -o moc_ObjProperty.cpp

moc_OBPRMGUI.cpp: OBPRMGUI.h 
	$(MOC) OBPRMGUI.h -o moc_OBPRMGUI.cpp

moc_AddObjDialog.cpp: AddObjDialog.h
	$(MOC) AddObjDialog.h -o moc_AddObjDialog.cpp

moc_VizmoEditor.cpp: VizmoEditor.h
	$(MOC) VizmoEditor.h -o moc_VizmoEditor.cpp

moc_QueryGUI.cpp: QueryGUI.h
	$(MOC) QueryGUI.h -o moc_QueryGUI.cpp

-include Dependencies
