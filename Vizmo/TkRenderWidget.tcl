catch {load vtktcl}

# This script uses a vtkTkRenderWidget to create a
# Tk widget that is associated with a vtkRenderWindow.
source TkInteractor.tcl

# Load in standard bindings for interactor

# Create the GUI: two renderer widgets and a quit button
#
wm withdraw .
toplevel .top 
toplevel .a

frame .top.f1 

vtkTkRenderWidget .top.f1.r1 -width 300 -height 300 
    BindTkRenderWidget .top.f1.r1

vtkTkRenderWidget .a.canvas -width 300 -height 300 
    BindTkRenderWidget .a.canvas

button .top.btn  -text Quit -command exit

pack .top.f1.r1 -side left -padx 3 -pady 3 -fill both -expand t
pack .a.canvas -side left -padx 3 -pady 3 -fill both -expand t
pack .top.f1  -fill both -expand t
pack .top.btn -fill x

# Get the render window associated with the widget.
set renWin1 [.top.f1.r1 GetRenderWindow]
vtkRenderer ren1;
$renWin1 AddRenderer ren1;

set renWin2 [.a.canvas GetRenderWindow]
vtkRenderer ren2;
$renWin2 AddRenderer ren2;


# create a sphere source and actor
vtkSphereSource sphere
vtkPolyDataMapper   sphereMapper
    sphereMapper SetInput [sphere GetOutput]
    sphereMapper ImmediateModeRenderingOn
vtkLODActor sphereActor
    sphereActor SetMapper sphereMapper

# create the spikes using a cone source and the sphere source
#
vtkConeSource cone
vtkGlyph3D glyph
    glyph SetInput [sphere GetOutput]
    glyph SetSource [cone GetOutput]
    glyph SetVectorModeToUseNormal
    glyph SetScaleModeToScaleByVector
    glyph SetScaleFactor 0.25
vtkPolyDataMapper spikeMapper
    spikeMapper SetInput [glyph GetOutput]
    spikeMapper ImmediateModeRenderingOn
vtkLODActor spikeActor
    spikeActor SetMapper spikeMapper

# Add the actors to the renderer, set the background and size
#
ren1 AddActor sphereActor
ren1 AddActor spikeActor
ren1 SetBackground 0.1 0.2 0.4
$renWin1 SetSize 300 300

ren2 AddActor sphereActor
ren2 AddActor spikeActor
ren2 SetBackground 0.1 0.2 0.4
$renWin2 SetSize 300 300
