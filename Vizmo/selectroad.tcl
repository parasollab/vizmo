#script to create/handle the Display RoadMap dialog box


set rmdoneVar 0


#----------------------------------------------------------------------
# create the  Display RoadMap dialog box
#----------------------------------------------------------------------
proc DisplayRoadmapDialog {w value} {
	global genRoadNameVal EnvironmentFileName MapFileName
	#------------------------------------------------------------------------
	#display the roadmap creation options
	#------------------------------------------------------------------------
        set genRoadNameVal(Environment) $EnvironmentFileName
        set genRoadNameVal(RoadMap) $MapFileName
        #puts "MapFileName =  $MapFileName"
        #puts "genRoadNameval =  $genRoadNameVal(RoadMap)"

	frame $w.roadmapOptions -relief groove -bd 1 -height 75
	label $w.roadmapOptions.title -text "Display Roadmap "
	pack  $w.roadmapOptions.title -ipadx 5 -ipady 5 -pady 3 -side top -anchor w
	foreach i { RoadMap } {
		frame $w.roadmapOptions.f$i -width 40 
		label $w.roadmapOptions.f$i.l$i -text "$i" -width 20
		entry $w.roadmapOptions.f$i.e$i -textvariable genRoadNameVal(RoadMap) -width 40
		#entry $w.roadmapOptions.f$i.e$i -textvariable MapFileName -width 40
                $w.roadmapOptions.f$i.e$i delete 0 end
       		$w.roadmapOptions.f$i.e$i insert 0 $genRoadNameVal(RoadMap)
       		#$w.roadmapOptions.f$i.e$i insert 0 $genRoadNameVal(RoadMap)
		button $w.roadmapOptions.f$i.b$i -text Select -command "DisplayRoadOptions $i $w"
		pack $w.roadmapOptions.f$i.l$i -side left -fill y -ipadx 5 -anchor w 
		pack  $w.roadmapOptions.f$i.b$i $w.roadmapOptions.f$i.e$i -side right -fill y -ipadx 3
		pack $w.roadmapOptions.f$i -side top -padx 5 -fill both -pady 5 -ipadx 3 -ipady 5
	}
	frame $w.buttons 
	button $w.buttons.start -text Ok -width 10  -command "genRoadMapOnStart $w $value"
	button $w.buttons.cancel -text Cancel -command "genRoadMapOnCancel $w" -width 10
	pack  $w.buttons.start $w.buttons.cancel -side left -padx 5 -expand 1

	pack $w.roadmapOptions $w.buttons -side top -fill both -pady 5 -expand 1

	#------------------------------------------------------------------------
	# call functions to get values to be displayed 
	#------------------------------------------------------------------------
	#set result [ getGenRoadValues ]

	#return $result 
}

#----------------------------------------------------------------------
# main routine to create and display the Display RoadMap  dialog 
# called from main window
#----------------------------------------------------------------------

proc displayDisplayRoadMapDialog {value} {
    global rmdoneVar
    global mainwindow
    set parent $mainwindow
    grab $parent
    set w .genRoadmapDialog
    catch {destroy $w}
    toplevel $w
    wm title $w "Display Roadmap"
    wm iconname $w "Display Roadmap"
    wm geometry $w +100+100
    DisplayRoadmapDialog $w $value
    grab release $parent
}


#------------------------------------------------------------------------
# call functions to get values to be displayed 
#------------------------------------------------------------------------
proc getGenRoadValues {} {

	global genRoadNameVal

	# if window is not open yet these do not exist
	if { [ winfo exists .genRoadmapDialog] == 0 } {
		##puts stdout "genRoadmapDialog window unopen"
		return 
	}
	foreach {i j} { Environment 0 RoadMap 1} {
		set genRoadNameVal($i) [GetFileNames $j]
	}
}

#------------------------------------------------------------------------
# call functions to get new files if required
#------------------------------------------------------------------------

proc DisplayRoadOptions {type w} {
	global SUCCESS FAILURE genRoadNameVal EnvironmentFileTypes RoadMapFileTypes
	global mainwindow DefaultWorkingDir EnvironmentWorkingDir
	set genRoadEnv $genRoadNameVal(Environment)
	set fileroot [file rootname $genRoadEnv]
	set defaultMapFile $EnvironmentWorkingDir/$fileroot.map

	# need to find the filename to be displayed
	#puts stdout $type 
	if { $type == "Environment" } {
		set typeList $EnvironmentFileTypes
		set thisDir $DefaultWorkingDir
		set thisExt ".env"
		set filepath [tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		 -title "Display $type" -initialdir $thisDir -defaultextension $thisExt]

	} elseif { $type == "RoadMap" } {
		set typeList $RoadMapFileTypes
		set thisDir $EnvironmentWorkingDir
		set thisExt ".map"
		set filepath [tk_getSaveFile -filetypes $typeList -parent $mainwindow \
		 -title "Display $type" -initialfile $defaultMapFile -initialdir $thisDir -defaultextension $thisExt]

	} else {
		return $FAILURE
	}
	set filename [file tail $filepath]
	if {[string compare $filename "" ] != 0 } {
		set genRoadNameVal($type) $filepath
	}
	# this is neat way to bring your other dialog on top
	raise $w 
}
#------------------------------------------------------------------------
# call functions to generate a new roadmap
#------------------------------------------------------------------------

proc genRoadMapOnStart {w value} { 
	global rmdoneVar genRoadNameVal FAILURE
	# need to call the function to start the roadmap generation
	foreach i { RoadMap} {
		if {[string compare $genRoadNameVal($i) "" ] == 0 } {
			set result $FAILURE
			return $result
		}
	}
	set rmdoneVar 1
	destroy $w
	set result [RoadMapDisplay $value ]
}
#------------------------------------------------------------------------
# call functions to if cancel is clicked
#------------------------------------------------------------------------

proc genRoadMapOnCancel {w} {
	global rmdoneVar
	set rmdoneVar 1
	destroy $w
}
#------------------------------------------------------------------------
# actual function to process parameters and generate roadmap
#------------------------------------------------------------------------

proc RoadMapDisplay {value} {
	global genRoadNameVal MakemapExeName IsEnvironmentModifiedFlag TRUE SUCCESS
	global DataFilesDir mainwindow EnvironmentWorkingDir FAILURE DefaultDataFilesDir result
	global READY NORMAL WAIT GENERATING_MAP_FILE CANCEL NO_WRITE_PERMISSION
        global Renderer
	# check if user specified an env or mod file
	set genRoadMapfile $genRoadNameVal(RoadMap)


	#puts "geyik $genRoadMapfile";
        set result [ EnvironmentDisplayRoadmap $genRoadMapfile $value Renderer ]
        if { $result == $SUCCESS } { 
           #puts "oku duk\n"
	} else { 
           #puts "selamlar salamlar\n"
          }
	UpdateUI
}
	

proc RoadmapHide {} {
    global Renderer
    EnvironmentHideRoadmap Renderer 
    UpdateUI
}

