# **** The original copy is VizmoView.tcl3 Some procs have been changed
# **** temporarily to add screen capturing capabilities to vizmo.
# **** All changes are documented in this file

#! /pub/sphere1/vtk/bin/vtk.exe

set DISTRIBUTION_DIR "/pub/dsmft/VIZMO/Vizmo-1.601"
#set  filename "$DISTRIBUTION_DIR"
#append filename actrprop.tcl
#puts stdout $filename
#-------------------------------------------------------------------------
# procedure to load all required subsidiary files
# this needs to be global in scope in all global variables in these files
# remain global
#-------------------------------------------------------------------------
set env_name " "
if [catch {
        set filename $DISTRIBUTION_DIR/actrprop.tcl
        source $filename
        set filename $DISTRIBUTION_DIR/genquery.tcl
        source $filename
        set filename $DISTRIBUTION_DIR/genpath.tcl
        source $filename
        set filename $DISTRIBUTION_DIR/genroad.tcl
        source $filename
        set filename $DISTRIBUTION_DIR/selectroad.tcl
        source $filename
	set filename $DISTRIBUTION_DIR/capturevideo.tcl
	source $filename
        set filename $DISTRIBUTION_DIR/parameters.tcl
   	source $filename } execResult] {
		puts stderr "ERROR IN LOADING FILE $filename"
                exit 
		return $execResult
   }

#-------------------------------------------------------------------------
## GLOBAL VARIABLES - trying to put the main window and frames
#-------------------------------------------------------------------------

set SUCCESS 0
set FAILURE 1
set CANCEL 2
set TRUE 1
set FALSE 0 
set FORWARD 1
set REVERSE 0
set NORMALTIME 50
set QUICKTIME 50
set SWAP 0

# status messages
set READY 0
set READING_ENVIRONMENT_FILE 1
set READING_PATH_FILE 2
set READING_QUERY_FILE 3
set READING_OBJECT_FILE 4
set GENERATING_MAP_FILE 5
set GENERATING_PATH_FILE 6
set SAVING_FILE 7
set COPYING_OBJECT_FILES 8

#user error messages
set INVALID_FILE 1
set NO_WRITE_PERMISSION 2
set INVALID_ACTION_WHEN_PATH_OPEN 3

# cursor shapes
set NORMAL 0
set WAIT 1

set IsEnvironmentModifiedFlag 0

set EnvironmentFileName "Untitled.env"
set PathFileName "Untitled.path"
set MapFileName "Untitled.map"
set genRoadNameVal(RoadMap)  "salak.map"

set QueryFileName "Untitled.cfg"

set NumOfFrames	0
set CurrentFrame 0
set StopFlag $FALSE
set PlayDirection $FORWARD
set StatusMessage "Ready"
set DefaultCursor left_ptr
set SliderLabel "File Name: $PathFileName          No.of Frames = $NumOfFrames"
set SliderFrame [expr $CurrentFrame+1]
set SelectedActorIndex 0
set SelectedActorName ""
set SelectedActorXPos 0.0
set SelectedActorYPos 0.0
set SelectedActorZPos 0.0
set SelectedActorRoll 0.0
set SelectedActorPitch 0.0
set SelectedActorYaw 0.0
set TranslateStepSize 1
set PlayStepSize 1
set RotateStepSize 1
set TimerStep $NORMALTIME
set GlobalCenterOfViewX 0 
set GlobalCenterOfViewY 0
set GlobalCenterOfViewZ 0

set XViewLaunched 0

set QueryExeName "/pub/dsmft/OBPRM/BASELINE-4.21/query"
set MakemapExeName "/pub/dsmft/OBPRM/BASELINE-4.21/makemap"
set EnvironmentWorkingDir [pwd]
set WorkingDir [pwd]
set DefaultObjectFilesDir "/pub/dsmft/burc/vizmo"
set ObjFileDir $WorkingDir
set PathDir $WorkingDir
# **** new ****
set CameraDir $WorkingDir

set EnvironmentFileTypes {
	{"Environment Files"    {".env"}}
   	{"All Files"		{"*.*"}}
}

set ActorFileTypes {
	{"Actor Files"      	{"*.g"}}
   	{"All Files"		{"*.*"}}
}

set RoadMapFileTypes {
   {"Roadmap Files"      {"*.map"}}
   {"All Files"		{"*.*"}}
}

set QueryFileTypes {
   {"Query Files"      	{"*.query"}}
   {"All Files"		{"*.*"}}
}
set PathFileTypes {
   {"Path Files"      	{"*.path"}}
   {"All Files"		{"*.*"}}
}

# **** new ****
set CameraShotTypes {
   {"Camera Files"	{"*.ppm"}}
   {"All Files"		{"*.*"}}
}

#-------------------------------------------------------------------------
## Common routines to handle scrollable listboxes
#-------------------------------------------------------------------------

proc Scroll_Set { scrollbar geoCmd offset size } {
	if { $offset != 0.0 || $size != 1.0} {
   	eval $geoCmd 	; # make sure it's visible
      $scrollbar set $offset $size
   } else {
   	set manager [lindex $geoCmd 0 ]
      $manager forget $scrollbar ; #hide the scrollbar
   }
}

proc Scrolled_Listbox { f args } {
	listbox $f.list -xscrollcommand [ list Scroll_Set $f.xscroll \
   		[ list grid $f.xscroll -row 1 -column 0 -sticky we]] \
      -yscrollcommand [ list Scroll_Set $f.yscroll \
      	[ list grid $f.yscroll -row 0 -column 1 -sticky ns]]
   eval {$f.list configure} $args
   scrollbar $f.xscroll -orient horizontal -command [list $f.list xview]
   scrollbar $f.yscroll -orient vertical -command [list $f.list yview]
   grid $f.list $f.yscroll -sticky news
   grid $f.xscroll -sticky news
   grid rowconfigure $f 0 -weight 1
   grid columnconfigure $f 0 -weight 1
   return $f.list
}
#-------------------------------------------------------------------------
# common routines to check file times
#-------------------------------------------------------------------------
proc newer { file1 file2 } {
	if ![file exists $file2] {
		return 1
	} else {
		#Assume file 1 exists
		expr [file mtime $file1] > [file mtime $file2]
	}
}

#-------------------------------------------------------------------------
# common routines to update status bar 
#-------------------------------------------------------------------------
proc UpdateStatus {msgID} {
	global StatusMessage READY READING_ENVIRONMENT_FILE GENERATING_MAP_FILE
	global GENERATING_PATH_FILE READING_PATH_FILE READING_QUERY_FILE SAVING_FILE 
	global READING_OBJECT_FILE COPYING_OBJECT_FILES

	if { [string compare $msgID $READY] == 0} {
		set StatusMessage "Ready" 
	} elseif {[string compare $msgID $READING_ENVIRONMENT_FILE] == 0} {
		set StatusMessage "Reading Environment..."
	} elseif {[string compare $msgID $READING_PATH_FILE] == 0} {
		set StatusMessage "Reading Path..."
	} elseif {[string compare $msgID $READING_QUERY_FILE] == 0} {
		set StatusMessage "Reading Query..."
	} elseif {[string compare $msgID $READING_OBJECT_FILE] == 0} {
		set StatusMessage "Reading Object..."
	} elseif {[string compare $msgID $GENERATING_MAP_FILE] == 0} {
		set StatusMessage "Generating Map File ..."
	} elseif {[string compare $msgID $GENERATING_PATH_FILE] == 0} {
		set StatusMessage "Generating Path File ..."
	} elseif {[string compare $msgID $SAVING_FILE ] == 0} {
		set StatusMessage "Saving File ..."
	} elseif {[string compare $msgID $COPYING_OBJECT_FILES] == 0} {
		set StatusMessage "Copying object files to Data directory ..."
	}
	update idletasks	
}
#-------------------------------------------------------------------------
# common routines to update cursor
#-------------------------------------------------------------------------
proc UpdateCursor {msgID} {
	global DefaultCursor WAIT NORMAL mainwindow
	if { [string compare $msgID $WAIT] == 0} {
		$mainwindow configure -cursor watch
	} elseif { [string compare $msgID $NORMAL] == 0} {
		$mainwindow configure -cursor $DefaultCursor
	}
	update idletasks
}
#-------------------------------------------------------------------------
# common routines to display messages
#-------------------------------------------------------------------------
proc SendUserMessage {msgID} {
	global mainwindow NO_WRITE_PERMISSION INVALID_FILE INVALID_ACTION_WHEN_PATH_OPEN
	set dispMessage "Error! msg ID is $msgID"
        #puts "In SenduserMessage\n"
	if { [string compare $msgID $INVALID_FILE] == 0} {
		set dispMessage "Invalid File. Check Access Permissions or Object Files may be missing."
	} elseif  { [string compare $msgID $NO_WRITE_PERMISSION] == 0} {
		set dispMessage "Invalid Operation. Check Write Permissions" 
	} elseif  { [string compare $msgID $INVALID_ACTION_WHEN_PATH_OPEN] == 0} {
		set dispMessage "Path loaded. Cannot delete robot" 
	}
	set choice [tk_messageBox -title "Vizmo 3D Error" \
		-parent $mainwindow -type ok -default ok \
		-message $dispMessage \
		-icon error ]
}

#-------------------------------------------------------------------------
# main view created
#-------------------------------------------------------------------------

set mainwindow .
wm title $mainwindow "VIZMO 3D $EnvironmentFileName"
wm geometry $mainwindow 1200x880
wm minsize $mainwindow 640 480

#-------------------------------------------------------------------------
# main menu options
#-------------------------------------------------------------------------

frame .menu -relief raised -bd 2
menubutton .menu.environment -text "Environment" -menu .menu.environment.menu
menubutton .menu.actor -text "Object" -menu .menu.actor.menu 
menubutton .menu.roadmap -text "Roadmap" -menu .menu.roadmap.menu -state disabled
menubutton .menu.query -text "Query" -menu .menu.query.menu -state disabled
menubutton .menu.view -text "View" -menu .menu.view.menu
menubutton .menu.help -text "Help" -menu .menu.help.menu
pack .menu -side top -fill x
pack .menu.environment .menu.actor .menu.roadmap .menu.query .menu.view -side left
pack .menu.help -side right

#-------------------------------------------------------------------------
# Set up the commands that will appear on the menubar under the environment menu
#-------------------------------------------------------------------------

menu .menu.environment.menu -tearoff 0
.menu.environment.menu add command -label "New" -underline 0 \
	-command NewEnvironment
.menu.environment.menu add command -label "Open..." -underline 0 \
	-command OpenEnvironment
.menu.environment.menu add command -label "Save..." -underline 0 \
	-command SaveEnvironment 
.menu.environment.menu add command -label "Save As..." -underline 1 \
	-command SaveAsEnvironment

# **** new ****
.menu.environment.menu add command -label "Camera..." -underline 2 \
        -command SaveCameraShot
.menu.environment.menu add command -label "Video..." -underline 2 \
	-command SaveVideoClip

.menu.environment.menu add command -label "Close" -underline 1 \
	-command CloseEnvironment 
.menu.environment.menu add separator
.menu.environment.menu add command -label "Colorize" -command Colorize \
	-underline 0
.menu.environment.menu add command -label "Background" -command BackgroundColor \
	-underline 0
.menu.environment.menu add separator
.menu.environment.menu add command -label "Exit" -command ExitVizmo \
	-underline 0

#-------------------------------------------------------------------------
## Set up the commands that will appear on the menubar under the actor menu
#-------------------------------------------------------------------------

menu .menu.actor.menu -tearoff 0
.menu.actor.menu add command -label "Add..." -underline 0 \
	-command AddActor
.menu.actor.menu add command -label "Delete" -underline 0 \
	-command DeleteActor
.menu.actor.menu add command -label "Properties..." -underline 0 \
	-command ActorProperties

#-------------------------------------------------------------------------
## Set up the commands that will appear on the menubar under the roadmap menu
#-------------------------------------------------------------------------

menu .menu.roadmap.menu -tearoff 0
.menu.roadmap.menu add command -label "Generate..." -underline 0 \
	-command GenerateRoadMap
#.menu.roadmap.menu add cascade -label "Display..." -underline 0 \
        #-menu  .menu.roadmap.type
.menu.roadmap.menu add command -label "Display..." -underline 0 \
        -command DisplayRoadMapAll 
.menu.roadmap.menu add command -label "Hide..." -underline 0 \
        -command RoadmapHide

#menu .menu.roadmap.type -tearoff 0
#.menu.roadmap.type add command -label "Simplified" -underline 0 \
        #-command DisplayRoadMapSimplified 

#.menu.roadmap.type add command -label "All" -underline 0 \
#        -command DisplayRoadMapAll 


#-------------------------------------------------------------------------
## Set up the commands that will appear on the menubar under the path menu
#-------------------------------------------------------------------------

menu .menu.query.menu -tearoff 0
.menu.query.menu add command -label "Specify..." -underline 0 \
	-command CreateQuery 
.menu.query.menu add command -label "Edit Specification..." -underline 0 \
	-command EditQuery 
.menu.query.menu add command -label "Run..." -underline 0 \
	-command GeneratePath 
.menu.query.menu add command -label "Display" -underline 0 \
	-command LoadPath 
.menu.query.menu add check -label "Swapvolume" -underline 0 \
        -command "SwapVolumePath "
.menu.query.menu add command -label "Close Display" -underline 0 \
	-command ClosePath

#-------------------------------------------------------------------------
## Set up the commands that will appear on the menubar under the view menu
#-------------------------------------------------------------------------

menu .menu.view.menu -tearoff 0
.menu.view.menu add command -label "Normal" -underline 0 \
	-command ViewNormal
.menu.view.menu add command -label "Along Z" -underline 6 \
	-command ViewAlongZ
.menu.view.menu add command -label "Along X" -underline 6 \
	-command ViewAlongX
.menu.view.menu add command -label "Along Y" -underline 6 \
	-command ViewAlongY
#.menu.view.menu add command -label "Launch X" -underline 6 \
        #-command launchXView





set panelColor #EEE
#-------------------------------------------------------------------------
## Set up the commands that will appear on the menubar under the help menu
#-------------------------------------------------------------------------

menu .menu.help.menu -tearoff 0
.menu.help.menu add command -label "About VIZMO 3D..." -underline 0 \
	-command AboutVizmo

#-------------------------------------------------------------------------#
# toolbar below menu bar
#-------------------------------------------------------------------------

#frame .toolbar -height 32 -relief raised -borderwidth 2
frame .toolbar -height 44 -relief raised -borderwidth 2
	frame .toolbar.file -height 38 -width 200
           frame .toolbar.file.new  
	      image create photo filenew -file    $DISTRIBUTION_DIR/gif/new.gif
              label .toolbar.file.new.txt -text "New " -bg $panelColor
	      button .toolbar.file.new.img -width 35 -height 38  \
    	         -command NewEnvironment -borderwidth 0 -highlightthickness 2
	      .toolbar.file.new.img configure -image filenew
           pack .toolbar.file.new.img .toolbar.file.new.txt -side top 
 	   pack .toolbar.file.new

           frame .toolbar.file.open  
              image create photo fileopen -file \
				$DISTRIBUTION_DIR/gif/folder05.gif
              label .toolbar.file.open.txt -text "Open " -bg $panelColor
              button .toolbar.file.open.img -width 35 -height 38  \
                 -command OpenEnvironment -borderwidth 0 -highlightthickness 2
	      .toolbar.file.open.img configure -image fileopen
           pack .toolbar.file.open.img .toolbar.file.open.txt -side top
           pack .toolbar.file.open


           frame .toolbar.file.save  
              image create photo filesave -file \
                                $DISTRIBUTION_DIR/gif/diskett3.gif
              label .toolbar.file.save.txt -text "Save " -bg $panelColor
              button .toolbar.file.save.img -width 35 -height 38  \
                 -command SaveEnvironment -borderwidth 0 -highlightthickness 2
              .toolbar.file.save.img configure -image filesave
           pack .toolbar.file.save.img .toolbar.file.save.txt -side top
           pack .toolbar.file.save


	pack .toolbar.file.new .toolbar.file.open .toolbar.file.save \
		-side left -fill x

	frame .toolbar.actor -height 38 -width 200
           frame .toolbar.actor.insert
              image create photo actorinsert -file  \
			  $DISTRIBUTION_DIR/gif/solids.gif
              label .toolbar.actor.insert.txt -text "Insert " -bg $panelColor
              button .toolbar.actor.insert.img -width 35 -height 38  \
                 -command AddActor -borderwidth 0 -highlightthickness 2
              .toolbar.actor.insert.img configure -image actorinsert
           pack .toolbar.actor.insert.img .toolbar.actor.insert.txt -side top
           pack .toolbar.actor.insert

           frame .toolbar.actor.delete
              image create photo actordelete -file  \
                          $DISTRIBUTION_DIR/gif/trash.gif
              label .toolbar.actor.delete.txt -text "Delete " -bg $panelColor
              button .toolbar.actor.delete.img -width 35 -height 38  \
                 -command DeleteActor -borderwidth 0 -highlightthickness 2
              .toolbar.actor.delete.img configure -image actordelete
           pack .toolbar.actor.delete.img .toolbar.actor.delete.txt -side top
           pack .toolbar.actor.delete


	pack .toolbar.actor.insert .toolbar.actor.delete\
		-side left -fill x

	frame .toolbar.roadmap -height 38 -width 200
           frame .toolbar.roadmap.generate
              image create photo roadmapgenerate -file  \
                          $DISTRIBUTION_DIR/gif/onramp2.gif
              label .toolbar.roadmap.generate.txt -text "Generate R." \
                        -bg $panelColor
              button .toolbar.roadmap.generate.img -width 35 -height 38  \
                 -command GenerateRoadMap -state disabled -borderwidth 0 \
                  -highlightthickness 2
              .toolbar.roadmap.generate.img configure -image roadmapgenerate
           pack .toolbar.roadmap.generate.img .toolbar.roadmap.generate.txt \
                -side top
           pack .toolbar.roadmap.generate

	pack .toolbar.roadmap.generate -side left -fill x

	frame .toolbar.path -height 38 -width 200
          frame .toolbar.path.generate
              image create photo pathgenerate -file  \
                          $DISTRIBUTION_DIR/gif/path.gif
              label .toolbar.path.generate.txt -text "Generate Path  " \
                        -bg $panelColor
              button .toolbar.path.generate.img -width 35 -height 38  \
                 -command GeneratePath -state disabled -borderwidth 0 \
                  -highlightthickness 2
              .toolbar.path.generate.img configure -image pathgenerate
           pack .toolbar.path.generate.img .toolbar.path.generate.txt \
                -side top

          frame .toolbar.path.display
              image create photo pathdisplay -file  \
                          $DISTRIBUTION_DIR/gif/launch.gif
              label .toolbar.path.display.txt -text "Load Path  " \
                        -bg $panelColor
              button .toolbar.path.display.img -width 35 -height 38  \
                 -command LoadPath -state disabled -borderwidth 0 \
                  -highlightthickness 2
              .toolbar.path.display.img configure -image pathdisplay
           pack .toolbar.path.display.img .toolbar.path.display.txt \
                -side top

        pack .toolbar.path.generate .toolbar.path.display -side left -fill x




	frame .toolbar.view -height 38 -width 200
           frame .toolbar.view.normal
           frame .toolbar.view.front
           frame .toolbar.view.side
           frame .toolbar.view.top
	   button .toolbar.view.normal.img -width 35 -height 38 \
   	   -command ViewNormal -borderwidth 0 -highlightthickness 2
	   button .toolbar.view.front.img -width 35 -height 38 \
   	   -command ViewAlongZ -borderwidth 0 -highlightthickness 2
	   button .toolbar.view.side.img -width 35 -height 38  \
   	   -command ViewAlongX -borderwidth 0 -highlightthickness 2
	   button .toolbar.view.top.img -width 35 -height 38   \
   	   -command ViewAlongY -borderwidth 0 -highlightthickness 2
	   #button .toolbar.view.launchx -width 35 -height 38   \
   	   #-command launchXView
   
	   image create photo viewnormal -file \
			$DISTRIBUTION_DIR/gif/viewplane.gif
	   image create photo viewfront -file \
			$DISTRIBUTION_DIR/gif/frontview.gif
	   image create photo viewside -file \
			$DISTRIBUTION_DIR/gif/sideview.gif
	   image create photo viewtop -file\
			 $DISTRIBUTION_DIR/gif/topview.gif
	   .toolbar.view.normal.img configure -image viewnormal
	   .toolbar.view.front.img configure -image viewfront
	   .toolbar.view.side.img configure -image viewside
	   .toolbar.view.top.img configure -image viewtop
            label .toolbar.view.normal.txt -text "Reset V." \
                        -bg $panelColor
            label .toolbar.view.front.txt -text "Front" \
                        -bg $panelColor
            label .toolbar.view.side.txt -text "Side" \
                        -bg $panelColor
            label .toolbar.view.top.txt -text "Top" \
                        -bg $panelColor
            pack .toolbar.view.normal.img .toolbar.view.normal.txt \
                -side top
            pack .toolbar.view.front.img .toolbar.view.front.txt \
                -side top
            pack .toolbar.view.side.img .toolbar.view.side.txt \
                -side top
            pack .toolbar.view.top.img .toolbar.view.top.txt \
                -side top


 
	pack .toolbar.view.normal .toolbar.view.front .toolbar.view.side \
		.toolbar.view.top -side left -fill x

# **** new ****
frame .toolbar.capture -height 38 -width 200
	frame .toolbar.capture.camera
		image create photo capturecamera -file \
		    $DISTRIBUTION_DIR/gif/camera2.gif
      		label .toolbar.capture.camera.txt -text "Camera" -bg $panelColor
      		button .toolbar.capture.camera.img -width 35 -height 38 \
         	    -command SaveCameraShot -borderwidth 0 -highlightthickness 2
		.toolbar.capture.camera.img configure -image capturecamera
	pack .toolbar.capture.camera.img .toolbar.capture.camera.txt -side top

	frame .toolbar.capture.video
		image create photo capturevideo \
		    -file $DISTRIBUTION_DIR/gif/movie.gif
		label .toolbar.capture.video.txt -text "Video" -bg $panelColor
		button .toolbar.capture.video.img -width 35 -height 38 \
         	    -command SaveVideoClip -borderwidth 0 -highlightthickness 1	
		.toolbar.capture.video.img configure -image capturevideo
	pack .toolbar.capture.video.img .toolbar.capture.video.txt -side top

pack .toolbar.capture.camera .toolbar.capture.video -side left -fill x


pack .toolbar.file .toolbar.actor .toolbar.roadmap .toolbar.path  \
	.toolbar.view .toolbar.capture -side left -padx 10

pack .toolbar -side top  -fill x

#-------------------------------------------------------------------------
# control panel on left of main frame
#-------------------------------------------------------------------------
set labelWidth 6
frame .left -width 100 -bg $panelColor -relief groove -bd 1

label .left.heading -text "Current Selection" -bg $panelColor
		 pack .left.heading -side top -fill x -ipady 7

frame .left.status
	frame .left.status.left -bg $panelColor -height 200 -width 10
	frame .left.status.right -bg $panelColor -height 200
	pack .left.status.left .left.status.right -side left -ipady 4 -ipadx 4 -fill both -expand 1

	foreach { x y z } { actorName Name SelectedActorName actorX X SelectedActorXPos \
				actorY Y SelectedActorYPos actorZ Z SelectedActorZPos \
			actorR Roll SelectedActorRoll actorP Pitch SelectedActorPitch actorYw Yaw SelectedActorYaw } {
	  label .left.status.left.label$x -text $y -bg $panelColor  -width $labelWidth -justify right
	  label .left.status.right.val$x -textvariable $z -bg $panelColor -fg #f00
	  pack .left.status.left.label$x -side top -fill x -ipady 5 -expand 1
	  pack .left.status.right.val$x -side top -fill x -ipady 5 -expand 1

	}
pack .left.heading  .left.status -side top -fill x

#-------------------------------------------------------------------------
# actor transformation panel
#-------------------------------------------------------------------------

frame .left.transform -bg #EEE -width 200 -height 200 
   label .left.transform.lTranslate -text "Translate Object"
   frame .left.transform.ftranStep 
   	label .left.transform.ftranStep.lTStep -text "Step Size" -width 8
   	entry .left.transform.ftranStep.eTStep -textvariable TranslateStepSize -width 4 -bg #fff
	pack .left.transform.ftranStep.lTStep .left.transform.ftranStep.eTStep -side left \
		-ipady 4 -ipadx 4 -padx 5 -pady 2 -fill both -expand 1

   frame .left.transform.translate -bg #EEE -width 200 -height 200 \
	-bd 1 -relief raised
	# create the canvas
	canvas .left.transform.translate.can -width 75 -height 75 -bg #000 \
		-borderwidth 0 -highlightthickness 0

	# create the buttons which will appear around the canvas
      	button .left.transform.translate.negx -text -X -width 1 -height 1 -bg #fff \
		-command "ActorTranslate x -" 
      	button .left.transform.translate.posx -text +X -width 1 -height 1 -bg #fff \
		-command "ActorTranslate x +" 
      	button .left.transform.translate.negy -text -Y -width 1 -height 1 -bg #fff \
		-command "ActorTranslate y -" 
      	button .left.transform.translate.posy -text +Y -width 1 -height 1 -bg #fff \
		-command "ActorTranslate y +" 
      	button .left.transform.translate.negz -text -Z -width 1 -height 1 -bg #fff \
		-command "ActorTranslate z -" 
      	button .left.transform.translate.posz -text +Z -width 1 -height 1 -bg #fff \
		-command "ActorTranslate z +" 

	# use the grid geometry manager to place canvas and buttons 
   	grid  .left.transform.translate.negx -row 1 -column 0 -rowspan 3 \
		 -columnspan 1 -sticky news
   	grid  .left.transform.translate.posx -row 1 -column 4 -rowspan 3 \
		-columnspan 1 -sticky news
   	grid  .left.transform.translate.negy -row 4 -column 1 -rowspan 1\
		-columnspan 3 -sticky news
   	grid  .left.transform.translate.posy -row 0 -column 1 -rowspan 1\
		-columnspan 3 -sticky news
   	grid  .left.transform.translate.negz -row 0 -column 4 -rowspan 1 \
		-columnspan 1 -sticky news
   	grid  .left.transform.translate.posz -row 4 -column 0 -rowspan 1 \
		-columnspan 1 -sticky news
   	grid  .left.transform.translate.can -row 1 -column 1 -columnspan 3 \
		-rowspan 3 -sticky news


   label .left.transform.lRotate -text "Rotate Object"
   frame .left.transform.frotStep
   	label .left.transform.frotStep.lRStep -text "Step Size" -width 8
   	entry .left.transform.frotStep.eRStep -textvariable RotateStepSize -width 3 -bg #fff
	pack .left.transform.frotStep.lRStep .left.transform.frotStep.eRStep -side left \
		-ipady 4 -ipadx 4 -padx 5 -pady 2 -fill both -expand 1

   frame .left.transform.rotate  -bg #EEE -bd 1 -relief raised \
	-width 150 -height 150
      	# create the canvas
	canvas .left.transform.rotate.can  -width 100 -height 100 -bg #000 \
		-borderwidth 0 -highlightthickness 0
		
	# create the buttons which will appear around the canvas
      	button .left.transform.rotate.negx -text -X -width 1 -height 1 -bg #fff \
		-command "ActorRotate x -" 
      	button .left.transform.rotate.posx -text +X -width 1 -height 1 -bg #fff \
		-command "ActorRotate x +" 
      	button .left.transform.rotate.negy -text -Y -width 1 -height 1 -bg #fff \
		-command "ActorRotate y -" 
      	button .left.transform.rotate.posy -text +Y -width 1 -height 1 -bg #fff \
		-command "ActorRotate y +" 
      	button .left.transform.rotate.negz -text -Z -width 1 -height 1 -bg #fff \
		-command "ActorRotate z -" 
      	button .left.transform.rotate.posz -text +Z -width 1 -height 1 -bg #fff \
		-command "ActorRotate z +" 

      	# use the grid geometry manager to place canvas and buttons 
   	grid  .left.transform.rotate.negx -row 2 -column 3 -rowspan 1 \
		-columnspan 1 -sticky news
     	grid  .left.transform.rotate.posx -row 3 -column 3 -rowspan 1  \
		-columnspan 1 -sticky news
      	grid  .left.transform.rotate.negy -row 0 -column 0 -rowspan 1 \
		-columnspan 1 -sticky nes
      	grid  .left.transform.rotate.posy -row 0 -column 1 -rowspan 1 \
		-columnspan 1 -sticky nws
      	grid  .left.transform.rotate.negz -row 1 -column 3 -rowspan 1 \
		-columnspan 1 -sticky news
      	grid  .left.transform.rotate.posz -row 0 -column 2 -rowspan 1 \
		-columnspan 1 -sticky nes
      	grid .left.transform.rotate.can -row 1 -column 0 -columnspan 3 \
   		-rowspan 3 -sticky news

	

pack   .left.transform.lTranslate .left.transform.ftranStep -side top -ipadx 5 -ipady 5 -anchor w
pack   .left.transform.translate -side top -padx 5 -pady 2
pack   .left.transform.rotate -side bottom -padx 5 -pady 2
pack   .left.transform.frotStep .left.transform.lRotate -side bottom -ipadx 5 -ipady 5 -anchor w
pack .left.transform -side top -fill both -ipady 15 

	# draw axes on canvas
	
	set tCanvas .left.transform.translate.can
	set tWidth [winfo reqwidth $tCanvas]
	set tHeight [winfo reqheight $tCanvas]
	#puts stdout "$tWidth $tHeight"
	set midX [ expr $tWidth / 2 ]
	set midY [ expr $tHeight /2 ]
      	.left.transform.translate.can create line $midX 0 $midX $tHeight -arrow both \
		 -fill #ff0 -arrowshape { 5 8 3 }
      	.left.transform.translate.can create line  0 $midY $tWidth $midY -arrow both \
		-fill #f00 -arrowshape { 5 8 3 }
      	.left.transform.translate.can create line 0 $tHeight $tWidth 0 -arrow both -fill #0f0 \
      			-arrowshape { 5 8 3 }


	# draw axes on canvas
	
	set rCanvas .left.transform.rotate.can
	set rWidth [winfo reqwidth $rCanvas]
	set rHeight [winfo reqheight $rCanvas]
	#puts stdout "width is $rWidth height is $rHeight"
	set startX [ expr $rWidth / 3 ]
	set endY [ expr $rHeight * 2 / 3 ]
	#puts stdout "startx is $startX endy is $endY"
      	.left.transform.rotate.can create line $startX $rHeight $startX 0 -arrow both -fill #ff0 \
      			-arrowshape { 5 8 3 }
      	.left.transform.rotate.can create line 0 $endY $rWidth $endY -arrow both -fill #f00 \
      			-arrowshape { 5 8 3 }
      	.left.transform.rotate.can create line 0 $rHeight $rWidth 0 -arrow both -fill #0f0 \
      			-arrowshape { 5 8 3 }
	set linePoints { [expr $startX-20] 0 $startX 20 [expr $startX+20] 0 }
	eval {.left.transform.rotate.can create line} $linePoints \
		{-tag line -joinstyle round -fill #fff -smooth true -arrow both}
	set linePoints { $rWidth [expr $endY-20] [expr $rWidth-20] $endY $rWidth [expr $endY+20]}
	eval {.left.transform.rotate.can create line} $linePoints \
		{-tag line -joinstyle round -fill #fff -smooth true -arrow both}
	set linePoints { [expr $rWidth-20] 0 [expr $rWidth-15] 15 $rWidth 20 }
	eval {.left.transform.rotate.can create line} $linePoints \
		{-tag line -joinstyle round -fill #fff -smooth true -arrow both}
	



#-------------------------------------------------------------------------
# right panel has canvas, status bar and vcr controls
#-------------------------------------------------------------------------

frame .right -width 100 -bg #000 -relief sunken

frame .right.status -bg SeaGreen2 -relief raised -borderwidth 1
label .right.status.message -textvariable StatusMessage -bg SeaGreen2
pack .right.status.message -side left

frame .right.vcr -height 30 -bg #555  -borderwidth 1
frame .right.vcr.left  -relief raised -bd 1
frame .right.vcr.right -relief raised -bd 1

image create photo Istop -file $DISTRIBUTION_DIR/gif/blue_stop.gif
image create photo Iforward -file $DISTRIBUTION_DIR/gif/play.gif
image create photo Iforwardstep -file $DISTRIBUTION_DIR/gif/step_forward.gif
image create photo Irewindstep -file $DISTRIBUTION_DIR/gif/step_back.gif
image create photo Irewind -file $DISTRIBUTION_DIR/gif/play_reverse.gif
image create photo Istart -file $DISTRIBUTION_DIR/gif/go_to_start.gif
image create photo Iend -file $DISTRIBUTION_DIR/gif/go_to_end.gif
image create photo Ifastforward -file $DISTRIBUTION_DIR/gif/fast_forward.gif
image create photo Ifastrewind -file $DISTRIBUTION_DIR/gif/fast_rewind.gif

frame .right.vcr.left.top
frame .right.vcr.left.bottom

      label .right.vcr.left.top.title -text "Path Display Control" -justify left
      pack .right.vcr.left.top.title -side left -padx 4 -fill x -expand 1
      label .right.vcr.left.top.sizeLabel -text "Step Size"  
      entry .right.vcr.left.top.sizeEntry  -textvariable PlayStepSize -width 4 -bg #fff -justify right
      pack .right.vcr.left.top.sizeLabel .right.vcr.left.top.sizeEntry -side left -ipady 4 -ipadx 4 -padx 5 -pady 2 -fill both -expand 1

      button .right.vcr.left.bottom.start -command PathGotoStart
	   .right.vcr.left.bottom.start configure -image Istart

      button .right.vcr.left.bottom.stop -command PathStopDisplay
		.right.vcr.left.bottom.stop configure -image Istop

      button .right.vcr.left.bottom.forward -command "PathPlayForward $NORMALTIME"
		.right.vcr.left.bottom.forward configure -image Iforward

      button .right.vcr.left.bottom.bfastforward -command "PathPlayForward $QUICKTIME"
	   .right.vcr.left.bottom.bfastforward configure -image Ifastforward
      
      button .right.vcr.left.bottom.forwardstep -command PathForwardStep
	   .right.vcr.left.bottom.forwardstep configure -image Iforwardstep

      
      button .right.vcr.left.bottom.rewind -command "PathPlayReverse $NORMALTIME"
	.right.vcr.left.bottom.rewind configure -image Irewind

      button .right.vcr.left.bottom.bfastrewind  -command "PathPlayReverse $QUICKTIME" 
      	.right.vcr.left.bottom.bfastrewind configure -image Ifastrewind
	
      button .right.vcr.left.bottom.rewindstep  -command PathRewindStep 
      	.right.vcr.left.bottom.rewindstep configure -image Irewindstep

      button .right.vcr.left.bottom.end -command PathGotoEnd
		.right.vcr.left.bottom.end configure -image Iend

      pack .right.vcr.left.bottom.stop \
	   .right.vcr.left.bottom.forward \
	   .right.vcr.left.bottom.bfastforward \
           .right.vcr.left.bottom.rewind \
	   .right.vcr.left.bottom.bfastrewind \
           .right.vcr.left.bottom.forwardstep \
           .right.vcr.left.bottom.rewindstep \
           .right.vcr.left.bottom.start \
    	   .right.vcr.left.bottom.end -side left

      pack .right.vcr.left.top -side top -fill x -expand 1
      pack .right.vcr.left.bottom -side top


     scale .right.vcr.right.slider -label $SliderLabel -from 1 -to $NumOfFrames  \
           -orient horizontal -variable SliderFrame -showvalue 1
     bind .right.vcr.right.slider <ButtonRelease-1> UpdateUI	
     pack  .right.vcr.right.slider -side top -fill both -expand 1

pack .right.vcr.left -side left
pack .right.vcr.right -side left -fill both -expand 1

pack .right.vcr -side bottom -fill x
pack .right.status -side bottom -fill x

#-------------------------------------------------------------------------
# drawing surface
#-------------------------------------------------------------------------
# This script uses a vtkTkRenderWidget to create a
# Tk widget that is associated with a vtkRenderWindow.
source $DISTRIBUTION_DIR/TkInteractor.tcl

frame .right.pane -bg #000 -height 400
pack .right.pane -side bottom -expand 1 -fill both
vtkTkRenderWidget .right.pane.canvas
    BindTkRenderWidget .right.pane.canvas
pack .right.pane.canvas -side top -fill both -expand 1


#Graphics objects
vtkCamera cameraNormal
	cameraNormal Azimuth 45
	cameraNormal Elevation 45
vtkCamera cameraNormalx
        cameraNormalx Azimuth 45
        cameraNormalx Elevation 45


vtkCamera cameraSide
	cameraSide Azimuth 90
vtkCamera cameraTop
	cameraTop Elevation 90
vtkCamera cameraFront



vtkLight light
vtkLight ll2ight
vtkRenderer  XViewRenderer
    XViewRenderer SetActiveCamera cameraNormalx
    XViewRenderer AddLight ll2ight
XViewRenderer SetBackground 0.0 0.0 0.0
vtkRenderer Renderer 
    Renderer SetActiveCamera cameraNormal
    Renderer AddLight light
#Get the render window associated with the widget.
set RenWin [.right.pane.canvas GetRenderWindow]
$RenWin AddRenderer Renderer
#$RenWin AddRenderer XViewRenderer
Renderer SetBackground 0.0 0.0 0.0
#set w .xview
       #set parent $mainwindow
        #toplevel $w
#
        #wm title $w "View Among X axis"
        #wm iconname $w "X-Axis"
        #wm geometry $w 1200x800
#
       #frame $w.pane -bg #000 -height 400
        #puts stdout "here2"
        #pack $w.pane  -side bottom -expand 1 -fill both
        #vtkTkRenderWidget $w.pane.canvas -width 800 -height 800
        #BindTkRenderWidget $w.pane.canvas
        #puts stdout "here3"
        #pack $w.pane.canvas -side top -fill both -expand t
        #puts stdout "here5"
        #set XViewWin $RenWin
        #$XViewWin AddRenderer XViewRenderer
        #$XViewWin AddRenderer Renderer




#vtkSphereSource sphere
#vtkPolyDataMapper   sphereMapper
    #sphereMapper SetInput [sphere GetOutput]
    #sphereMapper ImmediateModeRenderingOn
#vtkLODActor sphereActor
    #sphereActor SetMapper sphereMapper
#
# create the spikes using a cone source and the sphere source
#
#vtkConeSource cone
#vtkGlyph3D glyph
    #glyph SetInput [sphere GetOutput]
    #glyph SetSource [cone GetOutput]
    #glyph SetVectorModeToUseNormal
    #glyph SetScaleModeToScaleByVector
    #glyph SetScaleFactor 0.25
#vtkPolyDataMapper spikeMapper
    #spikeMapper SetInput [glyph GetOutput]
    #spikeMapper ImmediateModeRenderingOn
#vtkLODActor spikeActor
    #spikeActor SetMapper spikeMapper
#XViewRenderer AddActor sphereActor
#XViewRenderer AddActor spikeActor
#Renderer AddActor sphereActor
#Renderer AddActor spikeActor
#EnvironmentAddActor robot.g Renderer XViewRenderer
#XViewRenderer SetBackground 0.1 0.2 0.4



#create our axes for World Coordinates
vtkAxes worldAxes
	worldAxes SetOrigin 0 0 0
	worldAxes SetScaleFactor 50.0
vtkPolyDataMapper axesMapper
	axesMapper SetInput [ worldAxes GetOutput]
vtkActor worldAxesActor
	worldAxesActor SetMapper axesMapper
	worldAxesActor PickableOff
#Renderer AddActor worldAxesActor
# since we removed the axes -> dont reset without actors 
#Renderer ResetCamera

$RenWin StereoRenderOff
#$XViewWin StereoRenderOff
$RenWin Render
#$XViewWin Render

# assign a global picker too
vtkPicker thePicker

pack  .left  -side left -fill y
pack .right -side left -expand 1 -fill both

set DefaultCursor [$mainwindow cget -cursor]


if { $argc == 1 } {
    #puts stdout "salak= [lindex $argv 0]"
    set result [EnvironmentOpen [lindex $argv 0] $WorkingDir Renderer XViewRenderer]
    #puts stdout "geldi=$result"
    if { $result == $SUCCESS } {
            #Renderer ResetCamera
             #XViewRenderer ResetCamera
            #if { $XViewLaunched } { XViewRenderer ResetCamera }
     set EnvironmentFileName [EnvironmentGetFileName]
     set IsEnvironmentModifiedFlag 0
     set SelectedActorIndex [EnvironmentGetPickedActorIndex]
        set SelectedActorName [ActorPropsGetActorName $SelectedActorIndex]
                set SelectedActorXPos [ActorPropsGetOrientationValue 0 0]
                set SelectedActorYPos [ActorPropsGetOrientationValue 0 1]
                set SelectedActorZPos [ActorPropsGetOrientationValue 0 2]
                set SelectedActorRoll [ActorPropsGetOrientationValue 1 0]
                set SelectedActorPitch [ActorPropsGetOrientationValue 1 1]
                set SelectedActorYaw [ActorPropsGetOrientationValue 1 2]
                .menu.query configure -state normal
                .menu.roadmap configure -state normal
                .toolbar.path.generate.img configure -state normal
                .toolbar.path.display.img configure -state normal
                .toolbar.roadmap.generate.img configure -state normal
                set SliderFrame [ expr $CurrentFrame+1]

     wm title $mainwindow "VIZMO 3D $EnvironmentFileName"
        $RenWin Render


    }
    set EnvironmentLoadFlag 1
   set MapFileName [file rootname $EnvironmentFileName].map
set genRoadNameVal(RoadMap) $MapFileName

}

#-------------------------------------------------------------------------
# create a new environment
#-------------------------------------------------------------------------
proc NewEnvironment {} {
	global mainwindow
	set choice [tk_messageBox -title "Vizmo 3D" \
      	-parent $mainwindow -type yesnocancel -default yes \
      	-message "Create New Environment?" \
         -icon question ]
   if { [string compare $choice "yes" ] == 0 } {
	set result [ CloseEnvironment ]
   }
}

#-------------------------------------------------------------------------
# close current environment
#-------------------------------------------------------------------------
proc CloseEnvironment {} {
	global SUCCESS FAILURE CANCEL mainwindow IsEnvironmentModifiedFlag
   	global EnvironmentFileTypes EnvironmentFileName
   	set result $SUCCESS
	if { $IsEnvironmentModifiedFlag } {
   		# environment has been modified so save if required
 		set choice [tk_messageBox -title "Vizmo 3D" \
      			-parent $mainwindow -type yesnocancel -default yes \
      			-message "Save Current Environment?" \
         		-icon question ]
      		if { [string compare $choice "yes" ] == 0} {
      			set result [ SaveEnvironment ]
      		}
      		if { [string compare $choice "cancel" ] == 0} {
      			# don't continue with closing environment 
			# or saving current
         		set result $CANCEL
      		}
   	}
	if { $result == $SUCCESS } {
		set result [ClosePath]
		set result [QueryClose]
		set result [EnvironmentClose]
		set IsEnvironmentModifiedFlag 0
  		set EnvironmentFileName [EnvironmentGetFileName]
		#only update so that there is nothing to see
		UpdateUI
		
	}
}
#-------------------------------------------------------------------------
# opens existing environment files
#-------------------------------------------------------------------------

proc OpenEnvironment {} {
	global SUCCESS FAILURE CANCEL READING_ENVIRONMENT_FILE READY NORMAL WAIT
	global mainwindow IsEnvironmentModifiedFlag Renderer ObjFileDir PathDir
   	global EnvironmentFileTypes EnvironmentFileName Renderer RenWin WorkingDir
        global XViewRenderer XViewLaunched

	# **** new ****
	global CameraDir

   	set result $SUCCESS
	if { $IsEnvironmentModifiedFlag } {
   		# environment has been modified so save if required
 		set choice [tk_messageBox -title "Vizmo 3D" \
      			-parent $mainwindow -type yesnocancel -default yes \
      			-message "Save Current Environment Before Closing?" \
         		-icon question ]
      		#puts $choice
      		if { [string compare $choice "yes"] == 0 } {
      			set result [ SaveEnvironment ]
      		}
      		if { [string compare $choice "cancel"] == 0 } {
      			# don't continue with opening new environment or saving current
         			set result $FAILURE
      		}
	}
   	if { $result == $SUCCESS} {
   		set typeList $EnvironmentFileTypes
   		set filepath [tk_getOpenFile -filetypes $typeList -parent $mainwindow \
			 -title "Open Environment" -initialdir $WorkingDir -defaultextension ".env"]
         	set filename [file tail $filepath]
		set filext [file extension $filename]
		set fileroot [file rootname $filepath]
		set filedir [file dirname $filepath]
   		set MapFileName [file rootname $filepath].map
         	if { [string compare $filename ""] != 0 } {
		
			#close any open environments
			set result [ClosePath]
			set result [QueryClose]
			set result [EnvironmentClose]
			set IsEnvironmentModifiedFlag 0
			set EnvironmentFileName [EnvironmentGetFileName]
		                set WorkingDir [file dirname $filepath]
				set ObjFileDir $WorkingDir
				set PathDir $WorkingDir
			
				# **** new ****
				set CameraDir $WorkingDir
                                    
			UpdateStatus $READING_ENVIRONMENT_FILE
			UpdateCursor $WAIT
			if { [string compare $filext ".env"] == 0 } {
   				set result [ EnvironmentOpen $filepath $WorkingDir Renderer XViewRenderer]
			} elseif  {[string compare $filext ".mod"] == 0 } {
				# need to send the mod and ini file and replace 
				# the current mod file with env, file directory
				# is because we assume data is in same directory as mod file
				set inifilepath $fileroot.init 
				#puts stdout "$filepath $inifilepath"
				set result [ EnvironmentModOpen $filepath $inifilepath $filedir/ Renderer XViewRenderer]
				set filename $fileroot.env
			}
   			UpdateStatus $READY
			UpdateCursor $NORMAL
			if { $result == $SUCCESS } {
   				#Renderer ResetCamera
XViewRenderer ResetCamera
                                if { $XViewLaunched } { XViewRenderer ResetCamera }
				set EnvironmentFileName [EnvironmentGetFileName]

         		}
   		}
  	}
	UpdateUI
}

#-------------------------------------------------------------------------
# save current environment
#-------------------------------------------------------------------------

proc SaveEnvironment {} {
	global mainwindow SUCCESS FAILURE SAVING_FILE WAIT READY NORMAL
	global NO_WRITE_PERMISSION
   	global EnvironmentFileName EnvironmentFileTypes WorkingDir  ObjFileDir PathDir
	set result $SUCCESS
	set filepath $EnvironmentFileName
	set filename [file tail $filepath]
	if { $filename == "Untitled.env" } {
   	set typeList $EnvironmentFileTypes
		set filepath [tk_getSaveFile -filetypes $typeList -parent $mainwindow \
			-initialfile $EnvironmentFileName -defaultextension ".env" \
			-initialdir $WorkingDir -title "Save Environment"]
		set filename [file tail $filepath]
   	}
   	if { [ string compare $filename "Untitled.env"] != 0 } {
		if { [string compare $filename ""] != 0} {
			set fileroot [file rootname $filepath]
			set filedir [file dirname $filepath]
			set WorkingDir [file dirname $filepath]
			set ObjFileDir $WorkingDir
			set PathDir $WorkingDir
			if { [file writable $filedir] == 0 } {
				SendUserMessage $NO_WRITE_PERMISSION
				return $FAILURE	
			}
			set fileext [file extension $filepath]
			# if file currently loaded is a .mod file 
			if { [string compare $fileext ".mod"] == 0 } {
				set filepath $fileroot.env
			}
			set modPath $fileroot.mod
			set iniPath $fileroot.init
			#puts stdout "$filepath $modPath $iniPath"
			UpdateStatus $SAVING_FILE
			UpdateCursor $WAIT
			set result [ EnvironmentSave $filepath $modPath $iniPath]
			UpdateStatus $READY
			UpdateCursor $NORMAL
			
		}
   	} else {
   		set choice [tk_messageBox -title "Vizmo 3D" \
      		-parent $mainwindow -type ok -default ok \
      		-message "Illegal File Name" \
         	-icon error ]
      		set result $FAILURE
   	}
	if {$result == $SUCCESS } {
   		set IsEnvironmentModifiedFlag 0
        	set EnvironmentFileName [EnvironmentGetFileName]
   	} else {
      		# reset flags
      		set EnvironmentFileName [EnvironmentGetFileName]
   	}
	UpdateStatus $READY
	UpdateCursor $NORMAL
	UpdateUI
}

#-------------------------------------------------------------------------
# copy environment files
#-------------------------------------------------------------------------
proc SaveAsEnvironment {} {
	global mainwindow IsEnvironmentModifiedFlag PathDir ObjFileDir DefaultObjectFilesDir
   	global EnvironmentFileName EnvironmentFileTypes WorkingDir ObjFileDir
	global SUCCESS FAILURE SAVING_FILE WAIT READY NORMAL NO_WRITE_PERMISSION
	global COPYING_OBJECT_FILES
   	set result $SUCCESS
	set typeList $EnvironmentFileTypes
   	set filepath [tk_getSaveFile -filetypes $typeList -parent $mainwindow \
			-initialdir $WorkingDir -defaultextension ".env" \
			-title "Save Environment As" ]
	#puts stdout "save as $filepath"
	set filename [file tail $filepath]
      	if { [string compare $filename ""] != 0} {
		set fileroot [file rootname $filepath]
		set dirname [file dirname $filepath]
                set WorkingDir [file dirname $filepath]
                set ObjFileDir $WorkingDir 
                set PathDir $WorkingDir 
		if { [file writable $dirname] == 0 } {
			SendUserMessage $NO_WRITE_PERMISSION
			return $FAILURE	
		}
		
		set modPath $fileroot.mod
		set iniPath $fileroot.init
		UpdateStatus $SAVING_FILE
		UpdateCursor $WAIT
		set result [EnvironmentSave $filepath $modPath $iniPath]
		#puts stdout "result of save is $result"
		UpdateStatus $READY
		UpdateCursor $NORMAL
		# check if object files have to be saved too 
		set filetype [ file extension $EnvironmentFileName]
		if { [ string compare $filetype ".mod"] == 0} {
			set finaltype [file extension $filepath]
			if { [ string compare $finaltype ".mod"] == 0} {
				set ObjFileDir $dirname
			} elseif { [ string compare $finaltype ".env"] == 0} {
				set ObjFileDir $WorkDir
			}
			set choice [tk_messageBox -title "Vizmo 3D Convert PV File" \
				-parent $mainwindow -type yesno -default yes \
				-message "Copy object files to default data directory?" \
				-icon error ]
			if { [ string compare $choice "yes"] == 0 } {
				set sourceDir [file dirname $EnvironmentFileName]
				#puts stdout "Copying data files to $ObjFileDir"
				UpdateStatus $COPYING_OBJECT_FILES
				UpdateCursor $WAIT
				foreach match [glob -nocomplain "$sourceDir/*.g"] {
					file copy -force $match $ObjFileDir
				}
				UpdateStatus $READY
				UpdateCursor $NORMAL
				
			}
		}
		if { $result == $SUCCESS } {
      			set IsEnvironmentModifiedFlag 0
      			set EnvironmentFileName [EnvironmentGetFileName]
      		}
	}
	UpdateStatus $READY
	UpdateCursor $NORMAL
	UpdateUI
}



# **** new ****
#-------------------------------------------------------------------------
# save the camera shot
#-------------------------------------------------------------------------
proc SaveCameraShot {} {
	global mainwindow RenWin
	global CameraShotTypes WorkingDir CameraDir
	global SUCCESS FAILURE SAVING_FILE WAIT READY NORMAL NO_WRITE_PERMISSION

	set typeList $CameraShotTypes
	set filepath [tk_getSaveFile -filetypes $typeList \
		-parent $mainwindow -initialdir $CameraDir \
		-defaultextension ".ppm" -title "Save Camera Shot" ]
	#puts stdout "save as $filepath"
	set filename [file tail $filepath]
	if { [string compare $filename ""] != 0} {
		set fileroot [file rootname $filepath]
		set dirname [file dirname $filepath]
		set CameraDir [file dirname $filepath]
		if { [file writable $dirname] == 0 } {
			SendUserMessage $NO_WRITE_PERMISSION
			return $FAILURE
		}
		
		set modPath $fileroot.mod
		set iniPath $fileroot.init
		UpdateStatus $SAVING_FILE
		UpdateCursor $WAIT
		UpdateUI
		set result [$RenWin SetFileName $filepath]
		set result [$RenWin SaveImageAsPPM]
		#puts stdout "result of save is $result"
		UpdateStatus $READY
		UpdateCursor $NORMAL
	}
	UpdateStatus $READY
	UpdateCursor $NORMAL
	UpdateUI
}



# **** new ****
#-------------------------------------------------------------------------
# save the video clip
#-------------------------------------------------------------------------
proc SaveVideoClip {} {
	global mainwindow RenWin
	global CameraShotTypes WorkingDir CameraDir PathFileName
	global SUCCESS FAILURE SAVING_FILE WAIT READY NORMAL NO_WRITE_PERMISSION
	global PlayStepSize

	set stepSize $PlayStepSize
	set StartingFrame [PathGetCurrentFrame]
	set EndingFrame [PathGetNumberFrames]
	set result [displayCaptureVideoDialog $stepSize $StartingFrame $EndingFrame]
	UpdateStatus $READY
	UpdateCursor $NORMAL
	UpdateUI
}



#-------------------------------------------------------------------------
# exit the program
#-------------------------------------------------------------------------
proc ExitVizmo {} {
	set result [CloseEnvironment] 
	set result [ exit ]
}
proc Colorize {} {
        global IsEnvironmentModifiedFlag
        set result [RandomizeColor]
	UpdateUI
        set IsEnvironmentModifiedFlag 1

        
}
proc BackgroundColor {} {
        global mainwindow Renderer
        set color [tk_chooseColor -title "Choose a color" -parent $mainwindow \
        ]
    if [string compare $color ""] {
        set temp [winfo rgb $mainwindow $color]
        set  red [expr {[lindex $temp 0]/65535.0}]
        set green [expr {[lindex $temp 1]/65535.0}]
        set blue [expr {[lindex $temp 2]/65535.0}]


        
       Renderer SetBackground $red $green $blue
       UpdateUI
     }


    
}



#-------------------------------------------------------------------------
# add Actors into current environment
#-------------------------------------------------------------------------

proc AddActor {} {
	global SUCCESS FAILURE CANCEL mainwindow IsEnvironmentModifiedFlag
   	global ActorFileTypes Renderer RenWin ObjFileDir
        global XViewRenderer XViewLaunched
	global READING_OBJECT_FILE WAIT READY NORMAL
   	set result $SUCCESS
	set typeList $ActorFileTypes
   	set filepath [tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		 -title "Insert Object" -initialdir $ObjFileDir -defaultextension ".g"]
        set filename [file tail $filepath]
   	if {[string compare $filename "" ] != 0 } {
		UpdateStatus $READING_OBJECT_FILE
		UpdateCursor $WAIT
		# function to create actor and add to renderer
   		set result [ EnvironmentAddActor $filepath Renderer XViewRenderer]
		UpdateStatus $READY
		UpdateCursor $NORMAL
		
		if { $result == $SUCCESS } {
			set IsEnvironmentModifiedFlag 1
			#Renderer ResetCamera
			$RenWin Render
			XViewRenderer ResetCamera
  			if { $XViewLaunched } {
			XViewRenderer ResetCamera
                        }
		}

   	}
	# should ideally update all the UI
	UpdateUI
	set result $SUCCESS
}

#-------------------------------------------------------------------------
# delete Actors from current environment
#-------------------------------------------------------------------------
proc DeleteActor {} {
	global SUCCESS FAILURE CANCEL mainwindow IsEnvironmentModifiedFlag Renderer RenWin
   	global ActorFileTypes NumOfFrames TRUE INVALID_ACTION_WHEN_PATH_OPEN
        global XViewRenderer  XViewLaunched
	set result $SUCCESS 
	# check that if any paths are open, robots are not deleted
	if { $NumOfFrames > 0 } {
		if { [ActorPropsGetIsRobot] == $TRUE } {
			SendUserMessage $INVALID_ACTION_WHEN_PATH_OPEN
			return $result
		}
	}
	set result [ EnvironmentDeleteActor]
	if { $result == $SUCCESS } {
		set IsEnvironmentModifiedFlag 1
		#Renderer ResetCamera
		$RenWin Render
                        XViewRenderer ResetCamera

                if { $XViewLaunched } {
                        XViewRenderer ResetCamera
                        }

	}
	# should ideally update all the UI
	UpdateUI
	set result $SUCCESS
}


#------------------------------------------------------
# display the Actor Properties dialog box and handle it
#------------------------------------------------------

proc ActorProperties {} {
	set result [displayActorPropsDialog]
	UpdateUI
	return $result
}
#------------------------------------------------------
# transform the Actor by translation 
#------------------------------------------------------
proc ActorTranslate { dirn pos} {
	global SUCCESS FAILURE TranslateStepSize IsEnvironmentModifiedFlag
	set IsEnvironmentModifiedFlag 1
	#puts stdout "step size is $pos$TranslateStepSize"
	switch -exact -- $dirn {
		x { set result [ActorPropsSetOrientationValue 0 $pos$TranslateStepSize 0.0 0.0 ] }
		y { set result [ActorPropsSetOrientationValue 0 0.0 $pos$TranslateStepSize 0.0 ] }
		z { set result [ActorPropsSetOrientationValue 0 0.0 0.0 $pos$TranslateStepSize] }
		default { set result $FAILURE }
	}
	UpdateUI
	return $result
	
}
#------------------------------------------------------
# transform the Actor by rotation 
#------------------------------------------------------
proc ActorRotate { dirn pos } {
	global SUCCESS FAILURE RotateStepSize IsEnvironmentModifiedFlag
	set IsEnvironmentModifiedFlag 1
	#puts stdout "step size is $pos$RotateStepSize"
	switch -exact -- $dirn {
		x { set result [ActorPropsSetOrientationValue 1 $pos$RotateStepSize 0.0 0.0 ] }
		y { set result [ActorPropsSetOrientationValue 1 0.0 $pos$RotateStepSize 0.0 ] }
		z { set result [ActorPropsSetOrientationValue 1 0.0 0.0 $pos$RotateStepSize] }
		default { set result $FAILURE }
	}
	UpdateUI
	return $result
}



#-------------------------------------------------------------------------
# display the Generate RoadMap dialog box and proceed to generate roadmap
#-------------------------------------------------------------------------
proc GenerateRoadMap {} {
	global mainwindow
   	set result [displayGenerateRoadMapDialog]
	UpdateUI
   	return $result
}
#-------------------------------------------------------------------------
# display the Select RoadMap dialog box and proceed to generate roadmap
#-------------------------------------------------------------------------
proc DisplayRoadMapSimplified {} {
        global mainwindow
        set result [displayDisplayRoadMapDialog 0]
        UpdateUI
        return $result
}

proc DisplayRoadMapAll {} {
        global mainwindow
        set result [displayDisplayRoadMapDialog 1]
        UpdateUI
        return $result
}



#-------------------------------------------------------------------------
# display the Generate Query dialog box and proceed to specify configurations
#-------------------------------------------------------------------------
proc CreateQuery {} {
	global mainwindow SUCCESS
	set result $SUCCESS
	# must also create the query object
   	set result [QueryInitialize]
   	if { $result == $SUCCESS } {
		#puts stdout "Query initialized"
		set result [displayGenerateQueryDialog]
   	} else {
		#puts stdout "Query not initialized"
	}
	UpdateUI
	return $result
}

#-------------------------------------------------------------------------
# allow user to load a query file and edit and save it
#-------------------------------------------------------------------------
proc EditQuery {} {
	global QueryFileTypes mainwindow SUCCESS EnvironmentWorkingDir
	global READING_QUERY_FILE WAIT READY NORMAL
	set result $SUCCESS 
	set typeList $QueryFileTypes
   	set filepath [ tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		-title "Open Query" -initialdir $EnvironmentWorkingDir -defaultextension ".cfg"]
        set filename [file tail $filepath]
       	if { [string compare $filename ""] != 0} {
		set result [ QueryInitialize ]
		if { $result == $SUCCESS} {
			UpdateStatus $READING_QUERY_FILE
			UpdateCursor $WAIT
   			set result [ QueryOpen $filepath ]
			UpdateStatus $READY
			UpdateCursor $NORMAL
   			if { $result == $SUCCESS } {
   				set result [displayGenerateQueryDialog]
			}
		}
	}
	UpdateUI
	return $result
}

#-------------------------------------------------------------------------
# update the status of the Path Animation Panel
#-------------------------------------------------------------------------
proc UpdateAnimationPanel {} {
	global NumOfFrames CurrentFrame SliderLabel PathFileName NumOfFrames
	set NumOfFrames [PathGetNumberFrames]
	set CurrentFrame [ PathGetCurrentFrame ]
	set SliderFrame [ expr $CurrentFrame+1]
	set SliderLabel "File Name: $PathFileName          No.of Frames = $NumOfFrames"
	if { [winfo exists .right.vcr.right.slider]} {
		pack forget .right.vcr.right.slider
		destroy .right.vcr.right.slider
		scale .right.vcr.right.slider -label $SliderLabel \
			-from 1 -to $NumOfFrames -orient horizontal -variable SliderFrame \
			-showvalue 1
     		pack .right.vcr.right.slider -side top -fill both -expand 1
	}
}
#-------------------------------------------------------------------------
# close any open Path
#-------------------------------------------------------------------------
proc ClosePath {} {
	global PathFileName Renderer
        HideSwapVolume Renderer
	set result [ PathClose]
	set PathFileName "Untitled.path"
	UpdateAnimationPanel
	UpdateUI
}
#-------------------------------------------------------------------------
# initialize and animate the selected path file
#-------------------------------------------------------------------------

proc DisplayPath {filepath} {
	global PathFileName SUCCESS NumOfFrames CurrentFrame SliderLabel NORMALTIME
	global READING_PATH_FILE WAIT NORMAL READY .right.vcr.right.slider
	set PathFileName [file tail $filepath]
	set result $SUCCESS
        #puts stdout "Displaying"
   	if { [string compare $PathFileName ""] != 0} {
     	    if { [string compare $PathFileName "Untitled.path"] != 0 } {
			set result [ PathInitialize ]
		if { $result == $SUCCESS } {
			UpdateStatus $READING_PATH_FILE
			UpdateCursor $WAIT
			set result [ PathOpen $filepath ]
			UpdateStatus $READY
			UpdateCursor $NORMAL
			if { $result == $SUCCESS } {
				#puts stdout "path loaded correctly"
				UpdateAnimationPanel
				bind .right.vcr.right.slider <ButtonRelease-1> UpdateUI	
				.right.vcr.right.slider configure -command PathStopAndGoToFrame
				#updation is done in this function
				set result [PathPlayForward $NORMALTIME]
			}
		}
          }
   	}
	return $result
}


#-------------------------------------------------------------------------
# load the selected Path and proceed to animate
#-------------------------------------------------------------------------
proc SwapVolumePath {} {
   global SWAP Renderer
     if { $SWAP == 1 } { 
        set SWAP 0 
        HideSwapVolume Renderer
     } else  { 
        set SWAP 1 
        ShowSwapVolume Renderer}
   #puts stdout "Current state is $SWAP"
    
}
proc LoadPath {} {
	global PathFileTypes mainwindow SUCCESS FAILURE CANCEL
	global PathFileName WorkingDir EnvironmentFileName\
		SliderLabel NumOfFrames CurrentFrame \
		SliderFrame PathDir 
	set result $SUCCESS
	set typeList $PathFileTypes
   	set filepath [ tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		-title "Load Path" -initialdir $PathDir -defaultextension ".path"]

	if { [string compare $filepath ""] != 0 } {
		# check that environment is not newer than path
		if [newer $EnvironmentFileName $filepath ] {
			set choice [tk_messageBox -title "Vizmo 3D Path Display" \
      			-parent $mainwindow -type yesno -default yes \
      			-message "Environment has been modified! Path may not display correctly. Continue?" \
         			-icon warning ]
			if { [string compare $choice "no"] == 0 } {
				return $CANCEL
			}
		}
		# see if any path is already loaded, if so - close it
   		if { [string compare $PathFileName "Untitled.path"] != 0 } {
      		set result [ClosePath]
   		}
		set result [DisplayPath $filepath]
	}
   	return $result
}

#-------------------------------------------------------------------------
# display the Generate Path dialog box and proceed to generate paths
#-------------------------------------------------------------------------
proc GeneratePath {} {
	#puts stdout "Please wait..."
	set result [displayGeneratePathDialog]
	return $result
}

proc PathStopAndGoToFrame {pickedFrame} {
	global StopFlag 
	set old $StopFlag
	PathStopDisplay
	PathGoToFrame $pickedFrame
	set StopFlag $old
}

proc PathGoToFrame {pickedFrame} {
	global CurrentFrame SUCCESS
	##puts stdout "Path Going To Frame $pickedFrame"
	set result $SUCCESS
	set result [PathGoToStep $pickedFrame]
	set CurrentFrame [ PathGetCurrentFrame ]
	UpdateUI
	return $result
}


proc PathGotoStart {} {
	global CurrentFrame StopFlag TRUE PlayDirection REVERSE
	set StopFlag $TRUE
	#puts stdout "Path Restarted"
	set PlayDirection $REVERSE
	set result [PathSetDirection $PlayDirection]
	set CurrentFrame [ PathGetCurrentFrame ]
	set result [ PathGoToFrame 0 ]
	set CurrentFrame [ PathGetCurrentFrame ]
	UpdateUI
}

proc PathStopDisplay {} {
	global StopFlag TRUE
	set StopFlag $TRUE
}
		
proc NotStopped {} {
	global TRUE FALSE FORWARD NumOfFrames StopFlag CurrentFrame PlayDirection
	if { $StopFlag != $TRUE } {
		if {$PlayDirection == $FORWARD} {
			if { $CurrentFrame < [ expr $NumOfFrames-1] } {
				return $TRUE
			}
		} else {
			if { $CurrentFrame > 0 } {
				return $TRUE
			}
		}
	}
	return $FALSE
}

proc PathPlayForward {time} {
	global NumOfFrames StopFlag FALSE TRUE FORWARD CurrentFrame PlayDirection TimerStep timedUp PlayStepSize
	set StopFlag $TRUE
	set NumOfFrames [PathGetNumberFrames]	
	set CurrentFrame [ PathGetCurrentFrame ]
	set PlayDirection $FORWARD
	set result [PathSetDirection $PlayDirection]
	set TimerStep $time
	set StopFlag $FALSE
	while { [NotStopped] } {
		set timedUp $FALSE
		after $TimerStep { set timedUp $TRUE }
		vwait timedUp
		# set result [PathTakeAStep]
		set result [PathGoToFrame [expr $CurrentFrame+$PlayStepSize]]
		set CurrentFrame [ PathGetCurrentFrame ]
		UpdateUI
	}
	return $result
}

proc PathForwardStep {} {
	global CurrentFrame PlayDirection FORWARD StopFlag TRUE PlayStepSize
	set StopFlag $TRUE
	set PlayDirection $FORWARD
	set result [PathSetDirection $PlayDirection]
	set result [PathGoToFrame [expr $CurrentFrame+$PlayStepSize]]
	set CurrentFrame [ PathGetCurrentFrame ]
	UpdateUI 
	return $result
}

proc PathRewindStep {} {
	global CurrentFrame PlayDirection REVERSE StopFlag TRUE PlayStepSize
	set StopFlag $TRUE
	set PlayDirection $REVERSE
	set result [PathSetDirection $PlayDirection]
	set result [PathGoToFrame [expr $CurrentFrame-$PlayStepSize]]
	set CurrentFrame [ PathGetCurrentFrame ]
	UpdateUI 
	return $result
}

proc PathPlayReverse {time} {
	global NumOfFrames FALSE TRUE REVERSE StopFlag CurrentFrame PlayDirection TimerStep timedUp PlayStepSize
	set StopFlag $TRUE
	set NumOfFrames [PathGetNumberFrames]	
	set CurrentFrame [ PathGetCurrentFrame ]
	set PlayDirection $REVERSE
	set result [PathSetDirection $PlayDirection]
	set TimerStep $time
	set StopFlag $FALSE
	while { [NotStopped] } {
		set timedUp $FALSE
		after $TimerStep { set timedUp $TRUE }
		vwait timedUp
		#set result [PathTakeAStep]
		set result [PathGoToFrame [expr $CurrentFrame-$PlayStepSize]]
		set CurrentFrame [ PathGetCurrentFrame ]
		UpdateUI
	}
	return $result
}

proc PathGotoEnd {} {
	global CurrentFrame PlayDirection NumOfFrames REVERSE FORWARD SUCCESS StopFlag TRUE
	set StopFlag $TRUE
	#puts stdout "Path Going To Last Frame"
	set PlayDirection $FORWARD
	set result [PathSetDirection $PlayDirection]
	set CurrentFrame [ PathGetCurrentFrame ]
	#puts stdout $CurrentFrame
	set NumOfFrames [PathGetNumberFrames]	
	#puts stdout $NumOfFrames
	set result [PathGoToFrame [expr $NumOfFrames-1]]
	set CurrentFrame [ PathGetCurrentFrame ]
	UpdateUI
	return $result
}


#-------------------------------------------------------------------------
# display the view in perspective view
#-------------------------------------------------------------------------
proc ViewNormal {} {
	global Renderer cameraNormal  GlobalCenterOfViewX GlobalCenterOfViewY GlobalCenterOfViewZ
        global XViewRenderer
	Renderer SetActiveCamera cameraNormal
	XViewRenderer SetActiveCamera cameraNormal
	if { [EnvironmentGetNumberActors] > 0 } {
		Renderer ResetCamera
		XViewRenderer ResetCamera
	}
	UpdateUI
}

#-------------------------------------------------------------------------
# display the view along z axis
#-------------------------------------------------------------------------
proc ViewAlongZ {} {
	global Renderer cameraFront GlobalCenterOfViewX GlobalCenterOfViewY GlobalCenterOfViewZ
	Renderer SetActiveCamera cameraFront
	if { [EnvironmentGetNumberActors] > 0 } {
		Renderer ResetCamera
	}
	UpdateUI
}

#-------------------------------------------------------------------------
# display the view along x axis
#-------------------------------------------------------------------------
proc ViewAlongX {} {
	global Renderer cameraSide   GlobalCenterOfViewX GlobalCenterOfViewY GlobalCenterOfViewZ
    	Renderer SetActiveCamera cameraSide
	if { [EnvironmentGetNumberActors] > 0 } {
		Renderer ResetCamera
	}
	UpdateUI
}

#-------------------------------------------------------------------------
# display the view along y axis
#-------------------------------------------------------------------------
proc ViewAlongY {} {
	global Renderer cameraTop
    	Renderer SetActiveCamera cameraTop
	if { [EnvironmentGetNumberActors] > 0 } {
		Renderer ResetCamera
	}
	UpdateUI
}

proc xViewDestroyed {} {
  #puts stdout "x-view destroyed"

}

proc launchXView {} {
        global mainwindow XViewLaunched XViewRenderer
        global Renderer
        if { $XViewLaunched } { return }
        set parent $mainwindow
        set old [focus]
        set w .xview
        catch {destroy $w}
        
        toplevel $w
        wm title $w "View Among X axis"
        wm iconname $w "X-Axis"
        wm geometry $w 1200x800
        wm protocol $w WM_DELETE_WINDOW " set XViewLaunched 0 ; destroy $w "
        set XViewLaunched 1
        set doneVar 0
        #puts stdout "here2"
        vtkTkRenderWidget $w.canvas -width 800 -height 800
        BindTkRenderWidget $w.canvas
        #puts stdout "here3"
        pack $w.canvas -side top -fill both -expand t
        #puts stdout "here5"
        set XViewWin [$w.canvas GetRenderWindow]
        #$XViewWin AddRenderer XViewRenderer
        $XViewWin AddRenderer Renderer
	XViewRenderer SetBackground 0.0 0.0 0.0
        #puts stdout "here"

        #pack $w.canvas -side top -fill both -expand 1

        tkwait variable doneVar
         #bind $w.canvas <Destroy> {#puts stdout "okuz"}
        grab release $w
        focus $old
}

#-------------------------------------------------------------------------
# display the about vizmo dialog box
#-------------------------------------------------------------------------
proc AboutVizmo {} {
	UpdateUI
}


#-------------------------------------------------------------------------
# refresh display called whenever changes need to be reflected in
# all elements of the UI
#-------------------------------------------------------------------------

proc UpdateUI {} {
	global Renderer RenWin SliderLabel PathFileName 
	global XViewRenderer 
	global XViewLaunched
	global NumOfFrames CurrentFrame SliderFrame DefaultWorkingDir
	global SelectedActorIndex SelectedActorName SelectedActorXPos 
	global SelectedActorYPos SelectedActorZPos SelectedActorRoll 
	global IsEnvironmentModifiedFlag TRUE FALSE PathFileName 
	global mainwindow EnvironmentFileName EnvironmentWorkingDir
	global SelectedActorPitch SelectedActorYaw
	global .menu.query .menu.roadmap 
	global .toolbar.path.generate .toolbar.path.display .toolbar.roadmap.generate
	
	set EnvironmentFileName [EnvironmentGetFileName]
        
	wm title $mainwindow "VIZMO 3D: $EnvironmentFileName"

	
	# get Actor values - refreshes the actrProps dialog if open
	getValues

   	# get Robot values - refreshes the genQuery dialog if open
   	getAllRobotValues

	# get roadmap values - refreshes the genRoadMap dialog if open
	getGenRoadValues

	# get path values - refreshes the genPath dialog if open
	getGenPathValues

	# all the UI components should be refreshed
	if { [EnvironmentGetNumberActors] > 0 } {
		#puts "In loop"
		#Renderer UpdateActors ######## IRR
		#puts "In loop2"
                if { $XViewLaunched } { XViewRenderer UpdateActors }
		set SelectedActorIndex [EnvironmentGetPickedActorIndex]
		#puts stdout " SelectedActorIndex $SelectedActorIndex"
		set SelectedActorName [ActorPropsGetActorName $SelectedActorIndex] 
		#puts stdout "aptal1"
		set SelectedActorXPos [ActorPropsGetOrientationValue 0 0]
		#puts stdout "aptal2"
		set SelectedActorYPos [ActorPropsGetOrientationValue 0 1]
		#puts stdout "aptal3"
		set SelectedActorZPos [ActorPropsGetOrientationValue 0 2]
		#puts stdout "aptal4"
		set SelectedActorRoll [ActorPropsGetOrientationValue 1 0]
		#puts stdout "aptal5"
		set SelectedActorPitch [ActorPropsGetOrientationValue 1 1]
		#puts stdout "aptal6"
		set SelectedActorYaw [ActorPropsGetOrientationValue 1 2]
		#puts stdout "aptal7"
	} else {
		set SelectedActorIndex 0
		set SelectedActorName ""	
		set SelectedActorXPos 0.0
		set SelectedActorYPos 0.0
		set SelectedActorZPos 0.0
		set SelectedActorRoll 0.0
		set SelectedActorPitch 0.0
		set SelectedActorYaw 0.0
	}
	
	# we will assume that a user always has a valid environment open before
	# creating roadmaps, creating or editing queries, creating 
	# or displaying paths
        
	set validEnv [EnvironmentGetNumberRobots]
	#puts stdout "Num robots = $validEnv"
	if { $validEnv > 0 } {
		.menu.query configure -state normal
		.menu.roadmap configure -state normal
		.toolbar.path.generate.img configure -state normal
		.toolbar.path.display.img configure -state normal
		.toolbar.roadmap.generate.img configure -state normal
		set SliderFrame [ expr $CurrentFrame+1]
	} else {
		.menu.query configure -state disabled
		.menu.roadmap configure -state disabled
		.toolbar.path.generate.img configure -state disabled
		.toolbar.path.display.img configure -state disabled
		# see if any path is already loaded, if so - close it
   		if { [string compare $PathFileName "Untitled.path"] != 0 } {
      			set result [PathClose]
			UpdateAnimationPanel
		}
		.toolbar.roadmap.generate.img configure -state disabled
	}
	$RenWin Render
	#$XViewWin Render

}
