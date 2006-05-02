# script to create the Actor Properties dialog box
# this can be fine tuned later but you need to specify new variable names for 
# the buttons/labels/edit stuff which should be accessible from the rest of
# the program easily and not f.t.e.g !
# 02/25/98

#----------------------------------------------------------------------
# initialization of global variables - done when file is sourced
# have to be global in scope 
#----------------------------------------------------------------------
foreach i { Position Orientation Scale } {
	for {set j 0} {$j <= 2} {incr j} {
		# get the value to be displayed 
		set editVal($i$j) 1.0
		#puts $editVal($i$j)
	}
}
set numActors 0
#puts $numActors
set isRobot Obstacle
set isSolid Solid
#puts $isRobot
set actorColor #ffffffffffff
#puts $actorColor
set actorColorFrame 0
set ActorNameList {}
#puts $ActorNameList
set ActorFileName ""
		
#----------------------------------------------------------------------
# create the Actor Properties dialog box
#----------------------------------------------------------------------
proc ActorPropsDialog {w} {
	global editVal numActors isRobot actorColor actorColorFrame ActorNameList ActorFileName isSolid
	#puts $numActors
	#puts $isRobot
	#puts $actorColor
	#puts $ActorNameList
			
	frame $w.nameInfo -relief groove -bd 1 
	frame $w.actorInfo
	set temp $w.actorInfo
	frame $temp.typeInfo -relief groove -bd 1
	frame $temp.positionInfo -relief groove -bd 1 
	pack $temp.typeInfo $temp.positionInfo -side left -fill x -expand 1 \
		-padx 5 -pady 5
	frame $w.buttons 
	pack $w.nameInfo $temp $w.buttons -side top -expand 1 -padx 5 -pady 5

	#----------------------------------------------------------------------
	# create the actor names list box
	#----------------------------------------------------------------------

	frame  $w.nameInfo.top
	frame  $w.nameInfo.bottom
	pack  $w.nameInfo.top $w.nameInfo.bottom -side top -expand 1 -fill x -padx 5 -pady 5

	frame $w.nameInfo.top.left
	frame $w.nameInfo.top.right
	frame $w.nameInfo.top.farright
	label $w.nameInfo.top.left.name -text "Name"
	
	set ActorNameList \
   	[Scrolled_Listbox $w.nameInfo.top.right -width 20 -height 4 \
      	-setgrid 1 -bg #FFF -selectmode single ]

	# scrollbar $w.nameInfo.top.right.scroll -command "$w.nameInfo.right.list yview"
	# listbox $w.nameInfo.top.right.list -yscroll "$w.nameInfo.top.right.scroll set" \
	#	-setgrid 1 -height 4 -bg #FFF -selectmode single
	# set ActorNameList $w.nameInfo.top.right.list

	button $w.nameInfo.top.farright.bChangeActorName -text "Rename"
	button $w.nameInfo.top.farright.bAddActor -text "Add Object..." -command "AddActorAndRaise $w"
	button $w.nameInfo.top.farright.bDeleteActor -text "Delete Object" -command DeleteActor
	pack $w.nameInfo.top.farright.bChangeActorName \
		$w.nameInfo.top.farright.bAddActor $w.nameInfo.top.farright.bDeleteActor \
		-side top -fill x -padx 2 -pady 2
	# pack $w.nameInfo.top.right.scroll $w.nameInfo.top.right.list -side right -fill y
	pack $w.nameInfo.top.left.name -side top -fill x
	
	pack $w.nameInfo.top.left $w.nameInfo.top.right \
		$w.nameInfo.top.farright -side left -expand 1 -fill both -padx 5 -pady 5
	
	label $w.nameInfo.bottom.flabel -text "File" 
	label $w.nameInfo.bottom.fname -textvariable ActorFileName -width 30 -anchor w
	pack $w.nameInfo.bottom.flabel $w.nameInfo.bottom.fname -side left -padx 3 -ipadx 5

	# bind any selection in the list box
	bind $ActorNameList <ButtonRelease-1> \
		ActorSelected

	
	

	#----------------------------------------------------------------------
	# create the Robot/Obstacle Robot Color panel 
	#----------------------------------------------------------------------
	
	frame $temp.typeInfo.gp1 -bd 1 -relief raised
	frame $temp.typeInfo.gp2 -bd 1 -relief raised
	frame $temp.typeInfo.gp3 -bd 1 -relief raised

	label $temp.typeInfo.gp1.title -text Type -anchor w
	pack $temp.typeInfo.gp1.title -side top -fill x
	foreach i {Obstacle Robot } {
    		radiobutton $temp.typeInfo.gp1.b$i -text "$i" -variable isRobot \
	    		-relief flat -value $i -anchor w -width 10
    		pack $temp.typeInfo.gp1.b$i -side top -pady 2
	}
	#button $temp.typeInfo.gp1.bConfigure -text "Configure Robot..." \
	#	-command "editRobotConfiguration $temp"
		
	#pack $temp.typeInfo.gp1.bConfigure -side top -fill x -padx 2 -pady 2 

	label $temp.typeInfo.gp2.lColor -text Color -anchor w
	frame $temp.typeInfo.gp2.fColor -bg $actorColor \
		-relief raised -bd 1 -width 15 \
		-height 40
	set actorColorFrame $temp.typeInfo.gp2.fColor
	button $temp.typeInfo.gp2.bColorChange -text Change... -command \
		"setColor $w $temp.typeInfo.gp2.fColor bg -bg"

	pack $temp.typeInfo.gp2.lColor $temp.typeInfo.gp2.fColor \
    	    $temp.typeInfo.gp2.bColorChange -side top -fill x -padx 2 -pady 2 
 
        label $temp.typeInfo.gp3.title -text Type -anchor w
        pack $temp.typeInfo.gp3.title -side top -fill x
        foreach i {Wired Solid } {
                radiobutton $temp.typeInfo.gp3.b$i -text "$i" -variable isSolid\
                        -relief flat -value $i -anchor w -width 10
                pack $temp.typeInfo.gp3.b$i -side top -pady 2
        }


	pack $temp.typeInfo.gp1 $temp.typeInfo.gp2 $temp.typeInfo.gp3 -side top -fill both

	#----------------------------------------------------------------------
	# create the edit boxes for Position Orientation Scale entries
	#----------------------------------------------------------------------
	

	set ValsPosition [ list "X" "Y" "Z" ]
	set ValsOrientation [ list "Roll" "Pitch" "Yaw" ]
	set ValsScale [ list "In X" "In Y" "In Z" ]
	set k 0
	foreach i { Position Orientation Scale } {
		frame $temp.positionInfo.f$i -bd 1 -relief raised 
		pack $temp.positionInfo.f$i -side left -fill y -ipadx 5 -ipady 5 -expand 1
		label $temp.positionInfo.f$i.l$i -text "$i"	
		pack $temp.positionInfo.f$i.l$i -anchor w
		upvar 0 Vals$i labels 
		for {set j 0} {$j <= 2} {incr j} {
			label $temp.positionInfo.f$i.l$j -text [lindex $labels $j]
			entry $temp.positionInfo.f$i.e$j -width 20 -textvariable editVal($i$j) 
			pack  $temp.positionInfo.f$i.l$j $temp.positionInfo.f$i.e$j \
				-anchor w -side top -padx 5 -pady 5
		}
		incr k
	} 
	

	#------------------------------------------------------------------------
	#create the buttons on the lower panel
	#------------------------------------------------------------------------
	#send all the variables to be saved to the ok function
	button $w.buttons.ok -text "Modify Selected"  -width 20 -command "OnModify $w"
	button $w.buttons.cancel -text Close  -width 10 -command "OnClose $w"
	pack $w.buttons.ok $w.buttons.cancel -side left -expand 1

	
	#------------------------------------------------------------------------
	# call functions to get values to be displayed 
	#------------------------------------------------------------------------
	set result [ getValues ]
}

#----------------------------------------------------------------------------
# just calls the add actor function but also raises up the window afterwards
#----------------------------------------------------------------------------
proc AddActorAndRaise {w} {
	
	set result [ AddActor]
	raise $w
	
}
#----------------------------------------------------------------------
# procedure to display a dialog box to pick colors for a selected actor
#----------------------------------------------------------------------

proc setColor {w display name option} {
    global actorColor
    grab $w
    set initialColor [$display cget -$name]
    set color [tk_chooseColor -title "Choose a color" -parent $w \
	-initialcolor $initialColor]
    if [string compare $color ""] {
	$display config $option $color
	set actorColor $color
	#puts stdout $actorColor
    }
    grab release $w
}

#----------------------------------------------------------------------
# main routine to create and display the ActorProps dialog
# called from main window
#----------------------------------------------------------------------

proc displayActorPropsDialog {} {
    global mainwindow
    set parent $mainwindow
    grab $parent
    set w .actorProps
    catch {destroy $w}
    toplevel $w
    wm title $w "Object Properties"
    wm iconname $w "Object Properties"
    wm geometry $w +100+100
    ActorPropsDialog $w
    grab release $parent
}

#----------------------------------------------------------------------
# function called when an actor is selected in the listbox 
#----------------------------------------------------------------------

proc ActorSelected {} {
	global numActors ActorNameList
	set selected [$ActorNameList curselection]
	#puts stdout "Actor Selected at index $selected"
	# should only be one element in this list if single selection
	set index [ lindex $selected 0 ]
	#puts stdout $index
	set result [ EnvironmentSetPickedActorIndex $index ]
	# refresh displayed values
	getValues
	return $result
	
}


#----------------------------------------------------------------------
# function called to obtain all values again 
#----------------------------------------------------------------------

proc getValues {} {

	global numActors ActorNameList isRobot actorColor editVal actorColorFrame ActorFileName isSolid
	#puts "getValues entered"

	#-----------------------------
	# initialize default values 
	#-----------------------------
	set numActors 0
	set isRobot Obstacle
	set isSolid Solid
	set actorColor #ffffffffffff
	set ActorFileName ""

	# if window is not open yet these does not exist
	if { [ winfo exists .actorProps] == 0 } {
		#puts stdout "actorprops window unopen"
		return 
	}
	#default values
	$actorColorFrame config -bg $actorColor
	
	foreach i { Position Orientation Scale } {
		for {set j 0} {$j <= 2} {incr j} {
			# get the value to be displayed 
			set editVal($i$j) 1.0
		}
	}
	
	# get the number of actors currently available
	set numActors [EnvironmentGetNumberActors]
	#puts $numActors
	# delete all the current values
	if { [llength $ActorNameList] != 0} {
		$ActorNameList delete 0 $numActors
	}
	
	if { $numActors > 0 } {
	
		# get the current list of actor names to be displayed
		for {set i 0} {$i < $numActors} {incr i} {
				set actorName [ ActorPropsGetActorName $i ]
				$ActorNameList insert end $actorName
			}
		
	
		# highlight any current selection if any
		set selectedActorIndex [ EnvironmentGetPickedActorIndex ]
		$ActorNameList activate $selectedActorIndex
		# $ActorNameList selection anchor $selectedActorIndex
		$ActorNameList selection set $selectedActorIndex $selectedActorIndex
		
	
		# get the actors file name 
		set fName [ ActorPropsGetActorFileName $selectedActorIndex ]
		#puts stdout $fName
		set ActorFileName [ file tail $fName]

		# get the robot's configuration
		set isRobotFlag 0
		set isRobotFlag [ActorPropsGetIsRobot]
		if { $isRobotFlag == 0 } {
			set isRobot Obstacle
		} else {
			set isRobot Robot
		}

                set isSolidFlag 0
                set isSolidFlag [ActorPropsGetIsSolid]
                if { $isSolidFlag == 0 } {
                        set isSolid Wired
                } else {
                        set isSolid Solid
                }

		set actorColor [format "#%04x%04x%04x" \
			[ActorPropsGetCurrentColor 0] \
			[ActorPropsGetCurrentColor 1] \
			[ActorPropsGetCurrentColor 2] ]
		$actorColorFrame config -bg $actorColor
	
		# get the value to be displayed 
		set k 0
		foreach i { Position Orientation Scale } {
			for {set j 0} {$j <= 2} {incr j} {
				set editVal($i$j) [ActorPropsGetOrientationValue $k $j]
				#puts $editVal($i$j)	
			}
			incr k
		}	
	}
	update idletasks
	
}

#----------------------------------------------------------------------
# functions called when Ok or Cancel button is pressed 
#----------------------------------------------------------------------

proc OnModify {w} {
	global IsEnvironmentModifiedFlag editVal numActors isRobot actorColor ActorNameList isSolid
	# call the functions to save the newly changed values which are passed
	# to this function 

	#puts "OnModify entered"
	set IsEnvironmentModifiedFlag 1

	set index 0
	set temp Obstacle
	# set the robot's configuration
	if { $isRobot == $temp } {
		set returnVal($index) 0
	} else {
		set returnVal($index) 1
	}

	incr index
        set temp Wired
        if { $isSolid == $temp } {
                set returnVal($index) 0
        } else {
                set returnVal($index) 1
        }
        incr index


	# set the actor's color
	#puts $actorColor
	set temp [winfo rgb $w $actorColor]
	set returnVal($index) [lindex $temp 0]
	incr index
	set returnVal($index) [lindex $temp 1]
	incr index
	set returnVal($index) [lindex $temp 2]
	incr index
	
	
	# set the orientation value
	set k 0
	foreach i { Position Orientation Scale } {
		for {set j 0} {$j <= 2} {incr j} {
			set returnVal($index) $editVal($i$j) 
			incr index
		}
		incr k
	}
	set size [array size returnVal]
	for { set i 0} { $i < $size } { incr i} {
		#puts $returnVal($i)
	}
	set result [ActorPropsOnModifyClicked $returnVal(0) $returnVal(1)\
			$returnVal(2) $returnVal(3) $returnVal(4) $returnVal(5) \
			$returnVal(6) $returnVal(7) $returnVal(8) $returnVal(9) \
			$returnVal(10) $returnVal(11) $returnVal(12) \
                        $returnVal(13) ]
	UpdateUI
}

proc OnClose {w} {
	# don't update the values only close the window 
	set result [ActorPropsOnCloseClicked]
	destroy $w
	UpdateUI
}


