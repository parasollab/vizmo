// vizVTKButtonPage class declaration
#ifndef _H_vizVTKButtonPage
#define _H_vizVTKButtonPage

#include "vizBasics.h"
#include "vizAbstractObj.h"
#include "vtk.h"

#define MAX_BUTTONS 50

// Picking method not made a class method
void buttonPicker(void *);

class vizVTKButtonPage : public vizAbstractObj
{
  public:
    //constructors
    vizVTKButtonPage();
    ~vizVTKButtonPage();

  public:
    void(*userMethod)(void*);
    vtkActor        *buttonActors[MAX_BUTTONS];
    vizInt          numButtons;
    vizInt          option[MAX_BUTTONS];

    vtkRenderer               *renderer;
    vtkRenderWindowInteractor *iren;
    vtkRenderWindow           *renWin;
    vtkCamera                 *camera;
    vtkTextSource             *textSource;
    vtkPolyDataMapper         *buttonMapper;
    vtkPicker                 picker;

  public:
    //accessors
    vizInt GetNumberOfButtons() { return numButtons; }

    //methods
    void AddButton(char *command, vizInt opt);
    void SetUserMethod(void(*method)(void*));
    void Start();
};

#endif vizVTKButtonPage
