## Procedure should be called to set bindings and initialize variables
#
source $DISTRIBUTION_DIR/vtkInt.tcl

proc BindTkRenderWidget {widget} {
    bind $widget <Any-ButtonPress> {StartMotion %W %x %y}
    bind $widget <Any-ButtonRelease> {EndMotion %W %x %y}
    bind $widget <B1-Motion> {Rotate %W %x %y}
    bind $widget <B2-Motion> {Pan %W %x %y}
    bind $widget <B3-Motion> {Zoom %W %x %y}
    bind $widget <Shift-B1-Motion> {Pan %W %x %y}
    bind $widget <KeyPress-r> {Reset %W %x %y}
    bind $widget <KeyPress-u> {wm deiconify .vtkInteract}
    bind $widget <KeyPress-w> Wireframe
    bind $widget <KeyPress-s> Surface
    bind $widget <KeyPress-p> {Pick %W %x %y 0}
    bind $widget <Double-1> {Pick %W %x %y 0}
    bind $widget <Control-w> {Pick %W %x %y 1}
    bind $widget <Control-s> {Pick %W %x %y 2}
    bind $widget <Enter> {Enter %W %x %y}
    bind $widget <Leave> {focus $oldFocus}
}

# Global variable keeps track of whether active renderer was found
set RendererFound 0


#-------------------------------------------------------------------------
# Create event bindings
# this procedure is modified to be able to pick actors off the screen
# and set any one to wireframe or solid rendering
#-------------------------------------------------------------------------

proc Pick { widget x y type} {
	global CurrentRenderWindow CurrentRenderer thePicker
	set CurrentRenderWindow [$widget GetRenderWindow] 

   	# opposite directions renderwidget and renderwindow
  	set yval [ expr [lindex [$widget configure -height] 4] - $y]

	if { [thePicker Pick $x $yval 0.0 $CurrentRenderer] != 0} {
		#puts "actor picked"
		set foundActor [thePicker GetActor]
      		# sets the selected actor index
     		set result [EnvironmentPickActor $foundActor]
		if { $type == 1} {
			[$foundActor GetProperty] SetRepresentationToWireframe
		} elseif {$type == 2} {
			[$foundActor GetProperty] SetRepresentationToSurface
		}
   	}
	# display reflects the picked actor
	UpdateUI
}

proc Render {} {
    global CurrentCamera CurrentLight CurrentRenderWindow

    eval $CurrentLight SetPosition [$CurrentCamera GetPosition]
    eval $CurrentLight SetFocalPoint [$CurrentCamera GetFocalPoint]
    

    $CurrentRenderWindow Render
}

proc UpdateRenderer {widget x y} {
    global CurrentCamera CurrentLight 
    global CurrentRenderWindow CurrentRenderer
    global RendererFound LastX LastY
    global WindowCenterX WindowCenterY

    # Get the renderer window dimensions
    set WindowX [lindex [$widget configure -width] 4]
    set WindowY [lindex [$widget configure -height] 4]

    # Find which renderer event has occurred in
    set CurrentRenderWindow [$widget GetRenderWindow]
    set renderers [$CurrentRenderWindow GetRenderers]
    set numRenderers [$renderers GetNumberOfItems]

    $renderers InitTraversal; set RendererFound 0
    for {set i 0} {$i < $numRenderers} {incr i} {
	set CurrentRenderer [$renderers GetNextItem]
	set vx [expr double($x) / $WindowX]
	set vy [expr ($WindowY - double($y)) / $WindowY]
	set viewport [$CurrentRenderer GetViewport]
	set vpxmin [lindex $viewport 0]
	set vpymin [lindex $viewport 1]
	set vpxmax [lindex $viewport 2]
	set vpymax [lindex $viewport 3]
	if { $vx >= $vpxmin && $vx <= $vpxmax && \
	$vy >= $vpymin && $vy <= $vpymax} {
            set RendererFound 1
            set WindowCenterX [expr double($WindowX)*(($vpxmax - $vpxmin)/2.0\
                                + $vpxmin)]
            set WindowCenterY [expr double($WindowY)*(($vpymax - $vpymin)/2.0\
		                + $vpymin)]
            break
        }
    }
    
    set CurrentCamera [$CurrentRenderer GetActiveCamera]
    set lights [$CurrentRenderer GetLights]
    $lights InitTraversal; set CurrentLight [$lights GetNextItem]

    set LastX $x
    set LastY $y
}

proc Enter {widget x y} {
    global oldFocus

    set oldFocus [focus]
    focus $widget
    UpdateRenderer $widget $x $y
}

proc StartMotion {widget x y} {
    global CurrentCamera CurrentLight 
    global CurrentRenderWindow CurrentRenderer
    global LastX LastY
    global RendererFound

    UpdateRenderer $widget $x $y
    if { ! $RendererFound } { return }

    $CurrentRenderWindow SetDesiredUpdateRate 5.0
}

proc EndMotion {widget x y} {
    global CurrentRenderWindow
    global RendererFound

    if { ! $RendererFound } {return}
    $CurrentRenderWindow SetDesiredUpdateRate 0.01
    Render
}

proc Rotate {widget x y} {
    global CurrentCamera 
    global LastX LastY
    global RendererFound

    if { ! $RendererFound } { return }

    $CurrentCamera Azimuth [expr ($LastX - $x)]
    $CurrentCamera Elevation [expr ($y - $LastY)]
    $CurrentCamera OrthogonalizeViewUp
    set LastX $x
    set LastY $y

    Render
}

proc Pan {widget x y} {
    global CurrentRenderer CurrentCamera
    global WindowCenterX WindowCenterY LastX LastY
    global RendererFound

    if { ! $RendererFound } { return }

    set FPoint [$CurrentCamera GetFocalPoint]
        set FPoint0 [lindex $FPoint 0]
        set FPoint1 [lindex $FPoint 1]
        set FPoint2 [lindex $FPoint 2]

    set PPoint [$CurrentCamera GetPosition]
        set PPoint0 [lindex $PPoint 0]
        set PPoint1 [lindex $PPoint 1]
        set PPoint2 [lindex $PPoint 2]

    $CurrentRenderer SetWorldPoint $FPoint0 $FPoint1 $FPoint2 1.0
    $CurrentRenderer WorldToDisplay
    set DPoint [$CurrentRenderer GetDisplayPoint]
    set focalDepth [lindex $DPoint 2]

    set APoint0 [expr $WindowCenterX + ($x - $LastX)]
    set APoint1 [expr $WindowCenterY - ($y - $LastY)]

    $CurrentRenderer SetDisplayPoint $APoint0 $APoint1 $focalDepth
    $CurrentRenderer DisplayToWorld
    set RPoint [$CurrentRenderer GetWorldPoint]
        set RPoint0 [lindex $RPoint 0]
        set RPoint1 [lindex $RPoint 1]
        set RPoint2 [lindex $RPoint 2]
        set RPoint3 [lindex $RPoint 3]
    if { $RPoint3 != 0.0 } {
        set RPoint0 [expr $RPoint0 / $RPoint3]
        set RPoint1 [expr $RPoint1 / $RPoint3]
        set RPoint2 [expr $RPoint2 / $RPoint3]
    }

    $CurrentCamera SetFocalPoint \
      [expr ($FPoint0 - $RPoint0)/2.0 + $FPoint0] \
      [expr ($FPoint1 - $RPoint1)/2.0 + $FPoint1] \
      [expr ($FPoint2 - $RPoint2)/2.0 + $FPoint2]

    $CurrentCamera SetPosition \
      [expr ($FPoint0 - $RPoint0)/2.0 + $PPoint0] \
      [expr ($FPoint1 - $RPoint1)/2.0 + $PPoint1] \
      [expr ($FPoint2 - $RPoint2)/2.0 + $PPoint2]

    set LastX $x
    set LastY $y

    Render
}

proc Zoom {widget x y} {
    global CurrentCamera
    global LastX LastY
    global RendererFound

    if { ! $RendererFound } { return }

    set zoomFactor [expr pow(1.02,($y - $LastY))]

    if {[$CurrentCamera GetParallelProjection]} {
      set parallelScale [expr [$CurrentCamera GetParallelScale] * $zoomFactor];
      $CurrentCamera SetParallelScale $parallelScale;
    } else {
      set clippingRange [$CurrentCamera GetClippingRange]
      set minRange [lindex $clippingRange 0]
      set maxRange [lindex $clippingRange 1]
      $CurrentCamera SetClippingRange [expr $minRange / $zoomFactor] \
                                      [expr $maxRange / $zoomFactor]
      $CurrentCamera Dolly $zoomFactor
    }

    set LastX $x
    set LastY $y

    Render
}

proc Reset {widget x y} {
    global CurrentRenderWindow
    global RendererFound
    global CurrentRenderer
    global GlobalCenterOfViewX GlobalCenterOfViewY GlobalCenterOfViewZ
    # Get the renderer window dimensions
    set WindowX [lindex [$widget configure -width] 4]
    set WindowY [lindex [$widget configure -height] 4]

    # Find which renderer event has occurred in
    set CurrentRenderWindow [$widget GetRenderWindow]
    set renderers [$CurrentRenderWindow GetRenderers]
    set numRenderers [$renderers GetNumberOfItems]

    $renderers InitTraversal; set RendererFound 0
    for {set i 0} {$i < $numRenderers} {incr i} {
	set CurrentRenderer [$renderers GetNextItem]
	set vx [expr double($x) / $WindowX]
	set vy [expr ($WindowY - double($y)) / $WindowY]

	set viewport [$CurrentRenderer GetViewport]
	set vpxmin [lindex $viewport 0]
	set vpymin [lindex $viewport 1]
	set vpxmax [lindex $viewport 2]
	set vpymax [lindex $viewport 3]
	if { $vx >= $vpxmin && $vx <= $vpxmax && \
	$vy >= $vpymin && $vy <= $vpymax} {
            set RendererFound 1
            break
        }
    }
   
    if { $RendererFound } {
	
	set actors [$CurrentRenderer GetActors]
	
	if { $actors == {} } {
		return
	}
	$CurrentRenderer ResetCamera 
    	
	set currBounds { 0 0 0 0 0 0 }
    	set newBounds {}
    	#puts stdout	" current bounds $currBounds"
    	$actors InitTraversal
    	set actor [$actors GetNextItem]
    while { $actor != "" } {
	# get bounds of objects 
        set BBox [$actor GetBounds] 
	#puts stdout " bounding box $BBox"
	if { [lindex $currBounds 0 ] > [ lindex $BBox 0 ] } {
		lappend newBounds [lindex $BBox 0]
	} else {
		lappend newBounds [lindex $currBounds 0]
	}
	#puts stdout $newBounds
	if { [lindex $currBounds 1 ] < [ lindex $BBox 1 ] } {
		lappend newBounds [lindex $BBox 1]
	} else {
		lappend newBounds [lindex $currBounds 1]
		#puts stdout	$newBounds
	}
	if { [lindex $currBounds 2 ] > [ lindex $BBox 2 ] } {
		lappend newBounds [lindex $BBox 2]
	} else {
		lappend newBounds [lindex $currBounds 2]
	}
	#puts stdout	$newBounds
	if { [lindex $currBounds 3 ] < [ lindex $BBox 3 ] } {
		lappend newBounds [lindex $BBox 3]
	} else {
		lappend newBounds [lindex $currBounds 3]
	}
	#puts stdout	$newBounds
	if { [lindex $currBounds 4 ] > [ lindex $BBox 4 ] } {
		lappend newBounds [lindex $BBox 4]
	} else {
		lappend newBounds [lindex $currBounds 4]
	}
	#puts stdout	$newBounds
	if { [lindex $currBounds 5 ] < [ lindex $BBox 5 ] } {
		lappend newBounds [lindex $BBox 5]
	} else {
		lappend newBounds [lindex $currBounds 5]
	}
	set currBounds $newBounds
	set newBounds {}
	set actor [$actors GetNextItem]
    } 
    #puts stdout " new bounding box is $currBounds"
    set minx [lindex $currBounds 0]
    set maxx [lindex $currBounds 1]
    set miny [lindex $currBounds 2]
    set maxy [lindex $currBounds 3]
    set minz [lindex $currBounds 4]
    set maxz [lindex $currBounds 5]
    set GlobalCenterOfViewX [ expr [expr $minx + $maxx] / 2 ]
    set GlobalCenterOfViewY [ expr [expr $miny + $maxy] / 2 ]
    set GlobalCenterOfViewZ [ expr [expr $minz + $maxz] / 2 ]
    #puts stdout "$GlobalCenterOfViewX $GlobalCenterOfViewY $GlobalCenterOfViewZ"
    [$CurrentRenderer GetActiveCamera] SetFocalPoint $GlobalCenterOfViewX $GlobalCenterOfViewY $GlobalCenterOfViewZ
    set result [$CurrentRenderer ResetCamera]
	
    }

    Render
}

proc Wireframe {} {
    global CurrentRenderer

    set actors [$CurrentRenderer GetActors]

    $actors InitTraversal
    set actor [$actors GetNextItem]
    while { $actor != "" } {
        [$actor GetProperty] SetRepresentationToWireframe
        set actor [$actors GetNextItem]
    }

    Render
}

proc Surface {} {
    global CurrentRenderer

    set actors [$CurrentRenderer GetActors]

    $actors InitTraversal
    set actor [$actors GetNextItem]
    while { $actor != "" } {
        [$actor GetProperty] SetRepresentationToSurface
        set actor [$actors GetNextItem]
    }

    Render
}
