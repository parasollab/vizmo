# script to create the Generate Query dialog box
# 03/31/98

#----------------------------------------------------------------------
# initialization of global variables - done when file is sourced
# have to be global in scope 
#----------------------------------------------------------------------
foreach i { Position Orientation } {
	for {set j 0} {$j <= 2} {incr j} {
		# get the value to be displayed
		set RobotConfigVal($i$j) 1.0
		#puts $RobotConfigVal($i$j)
	}
}
set NumRobots 0
set RobotNameList {}
set NumRobotConfigs 0
set RobotConfigList {}
set QueryFileName "Untitled.cfg"
set RobotFileName "Untitled.g"

#----------------------------------------------------------------------
# create the Generate Query dialog box
#----------------------------------------------------------------------
proc GenerateQueryDialog {w} {
	global RobotConfigVal NumRobotConfigs NumRobots RobotConfigList
   	global RobotNameList QueryFileName

   	frame $w.selectRobotPanel -relief groove -bd 1
	frame $w.selectConfigPanel -relief groove -bd 1 
   	frame $w.editConfigPanel -relief groove -bd 1 
	frame $w.buttons 

	pack $w.selectRobotPanel $w.selectConfigPanel \
   		$w.editConfigPanel $w.buttons \
		-side top -expand 1 -padx 3 -pady 3

	#----------------------------------------------------------------------
	# create the robot names list box
	#----------------------------------------------------------------------

	frame  $w.selectRobotPanel.top
	frame  $w.selectRobotPanel.bottom
	pack  $w.selectRobotPanel.top $w.selectRobotPanel.bottom \
   		-side top -expand 1 -fill x -padx 5 -pady 5

	frame $w.selectRobotPanel.top.left
	frame $w.selectRobotPanel.top.right
	label $w.selectRobotPanel.top.left.name -text "Name"

   	set RobotNameList \
   	[Scrolled_Listbox $w.selectRobotPanel.top.right \
      	-width 20 -height 4 -setgrid 1 -bg #FFF -selectmode single ]

	# scrollbar $w.selectRobotPanel.top.right.scroll \
   	#	-command "$w.selectRobotPanel.right.list yview"
	# listbox $w.selectRobotPanel.top.right.list \
   	#	-yscroll "$w.selectRobotPanel.top.right.scroll set" \
	#	-setgrid 1 -height 4 -bg #FFF -selectmode single
	# set RobotNameList $w.selectRobotPanel.top.right.list
	# pack $w.selectRobotPanel.top.right.scroll \
   	#	$w.selectRobotPanel.top.right.list -side right -fill y

	pack $w.selectRobotPanel.top.left.name -side top -fill x

	pack $w.selectRobotPanel.top.left $w.selectRobotPanel.top.right \
		-side left -expand 1 -fill both -padx 5 -pady 5

	label $w.selectRobotPanel.bottom.flabel -text "File"
	label $w.selectRobotPanel.bottom.fname -textvariable RobotFileName -width 30 -anchor w
	pack $w.selectRobotPanel.bottom.flabel \
   		$w.selectRobotPanel.bottom.fname -side left -padx 3 -ipadx 5

	# bind any selection in the list box
	bind $RobotNameList <ButtonRelease-1> RobotSelected

	#----------------------------------------------------------------------
	# create the select Configuration panel
	#----------------------------------------------------------------------

   	frame $w.selectConfigPanel.left
	frame $w.selectConfigPanel.right
   	frame $w.selectConfigPanel.farright
   	label $w.selectConfigPanel.left.name -text "Name"

   	set RobotConfigList \
   	[Scrolled_Listbox $w.selectConfigPanel.right \
      	-width 20 -height 4 -setgrid 1 -bg #FFF -selectmode single ]

	
	button $w.selectConfigPanel.farright.bRenameGoal -text "Rename Goal" \
   		-command renameGoal
	button $w.selectConfigPanel.farright.bAddGoal -text "Add Goal" \
   		-command addGoal
	button $w.selectConfigPanel.farright.bDeleteGoal -text "Delete Goal" \
   		-command deleteGoal
	button $w.selectConfigPanel.farright.bnameGoal -text "Name Goal" \
   		-command nameGoal
	pack $w.selectConfigPanel.farright.bRenameGoal \
	$w.selectConfigPanel.farright.bAddGoal \
   	$w.selectConfigPanel.farright.bDeleteGoal \
   	$w.selectConfigPanel.farright.bnameGoal \
		-side top -fill x -padx 2 -pady 2

	pack $w.selectConfigPanel.left.name -side top -fill x

	pack $w.selectConfigPanel.left $w.selectConfigPanel.right \
		$w.selectConfigPanel.farright -side left -expand 1 -fill both -padx 5 -pady 5

	# bind any selection in the list box
	bind $RobotConfigList <ButtonRelease-1> GoalSelected



	#----------------------------------------------------------------------
	# create the edit boxes for Position Orientation entries
	#----------------------------------------------------------------------

   	frame $w.editConfigPanel.left
   	frame $w.editConfigPanel.right
   	pack $w.editConfigPanel.left $w.editConfigPanel.right \
   	-side left -expand 1 -fill both -padx 5 -pady 5
	set rValsPosition [ list "X" "Y" "Z" ]
	set rValsOrientation [ list "Roll" "Pitch" "Yaw" ]

	set k 0
	foreach i { Position Orientation } {
		frame $w.editConfigPanel.left.f$i -bd 1 -relief raised
		pack $w.editConfigPanel.left.f$i -side left -fill y -ipadx 5 -ipady 5 -expand 1
		label $w.editConfigPanel.left.f$i.l$i -text "$i"
		pack $w.editConfigPanel.left.f$i.l$i -anchor w
		upvar 0 rVals$i rlabels
		for {set j 0} {$j <= 2} {incr j} {
			label $w.editConfigPanel.left.f$i.l$j \
         			-text [lindex $rlabels $j]
			entry $w.editConfigPanel.left.f$i.e$j \
         			-width 6 -textvariable RobotConfigVal($i$j)
			pack  $w.editConfigPanel.left.f$i.l$j \
         			$w.editConfigPanel.left.f$i.e$j \
				-anchor w -side top -padx 5 -pady 5
		}
		incr k
	}
	button $w.editConfigPanel.right.saveConfig -text "Save Configuration" \
   		-command saveGoalConfiguration
   	button $w.editConfigPanel.right.setCurrent -text "Get Current Position" \
   		-command getCurrentPosition
   	pack  $w.editConfigPanel.right.saveConfig \
		$w.editConfigPanel.right.setCurrent \
		-side top -fill x -padx 5 -pady 5

	#------------------------------------------------------------------------
	#create the buttons on the lower panel
	#------------------------------------------------------------------------
	#send all the variables to be saved to the ok function
	button $w.buttons.saveQuery -text "Save Query" -width 20 -command "OnSaveQuery $w"
	button $w.buttons.closeQuery -text Close  -width 20 -command "OnCloseQuery $w"
	pack $w.buttons.saveQuery $w.buttons.closeQuery -side left -expand 1


	#------------------------------------------------------------------------
	# call functions to get values to be displayed
	#------------------------------------------------------------------------
	set result [ getAllRobotValues ]
}

#----------------------------------------------------------------------
# main routine to create and display the GenerateQuery dialog
# called from main window
#----------------------------------------------------------------------

proc displayGenerateQueryDialog {} {
    global mainwindow
    set parent $mainwindow
    grab $parent
    set w .genQueryDialog
    catch {destroy $w}
    toplevel $w
    wm title $w "Specify Query"
    wm iconname $w "Specify Query"
    wm geometry $w +100+100
    GenerateQueryDialog $w
    grab release $parent
}
#----------------------------------------------------------------------
# function called when a goal position is renamed
#----------------------------------------------------------------------
proc renameGoal {} {
	#puts stdout "Rename Goal"
	
	# should ideally update all the UI
	UpdateUI
	return $result
}

#----------------------------------------------------------------------
# function called when a goal position is modified
#----------------------------------------------------------------------
proc saveGoalConfiguration {} {
	global SUCCESS RobotConfigVal
	#puts stdout "Save Goal" 
	set result $SUCCESS
	set confIndex [QueryGetPickedConfigIndex]
	set robotIndex [QueryGetPickedRobotIndex]
	set result [QuerySetConfigValue $robotIndex $confIndex \
		$RobotConfigVal(Position0) $RobotConfigVal(Position1) $RobotConfigVal(Position2) \
		$RobotConfigVal(Orientation0) $RobotConfigVal(Orientation1) $RobotConfigVal(Orientation2)]
	if { $result != $SUCCESS } {
		return $result
	}
	# should ideally update all the UI
	UpdateUI
	return $result
}
#----------------------------------------------------------------------
# function called when a goal position is added
#----------------------------------------------------------------------
proc addGoal {} {
	#puts stdout "Adding Goal"
	set result [QueryAddGoal]
	# should ideally update all the UI
	UpdateUI
	return $result
}

#----------------------------------------------------------------------
# function called when a goal position is deleted
#----------------------------------------------------------------------
proc deleteGoal {} {
	global SUCCESS
	set confIndex [QueryGetPickedConfigIndex]
	set result $SUCCESS
	if { $confIndex != 0 } {
		set result [QueryDeleteGoal]
		# should ideally update all the UI
		UpdateUI
	}
	return $result
}

#----------------------------------------------------------------------
# function called when a goal position is selected in the listbox
# and we need to refresh the position info
#----------------------------------------------------------------------
proc getConfigValues {} {
	global RobotConfigVal
   	set robotIndex [QueryGetPickedRobotIndex]
   	set confIndex [QueryGetPickedConfigIndex]

   	#reinitialize
   	foreach i { Position Orientation } {
		for {set j 0} {$j <= 2} {incr j} {
			# get the value to be displayed
			set RobotConfigVal($i$j) 1.0
		}
	}
	# get the values to be displayed
   	set k 0
	foreach m { Position Orientation } {
		for {set n 0} {$n <= 2} {incr n} {
			set RobotConfigVal($m$n) [QueryGetConfigValue $robotIndex $confIndex $k $n]
			#puts $RobotConfigVal($m$n)
		}
		incr k
	}
   	# the actual refresh is done here
   	update idletasks
}

#----------------------------------------------------------------------
# function called when a robot is selected in the listbox and we
# need to refresh all its data
#----------------------------------------------------------------------
proc getRobotConfigs {} {

      	global RobotFileName NumRobotConfigs RobotConfigList
      	set RobotFileName "Untitled.g"
      	set NumRobotConfigs 0
      	set robotIndex [QueryGetPickedRobotIndex]

	# get the robot's file name
	set fName [ QueryGetRobotFileName ]
	set RobotFileName [ file tail $fName]

      	# get the number of goals currently available
	set NumRobotConfigs [QueryGetNumberRobotGoals $robotIndex]
	#puts stdout $NumRobotConfigs

	# delete all the current values
	if { [llength $RobotConfigList] != 0} {
		$RobotConfigList delete 0 end
	}

	if { $NumRobotConfigs > 0 } {
      		# get the current list of robot goals to be displayed
		for {set j 0} {$j < $NumRobotConfigs} {incr j} {
			set goalName [ QueryGetGoalName $robotIndex $j]
			$RobotConfigList insert end $goalName
		}

		# highlight any current selection if any
		set selectedConfigIndex [ QueryGetPickedConfigIndex ]
		$RobotConfigList activate $selectedConfigIndex
		$RobotConfigList selection set $selectedConfigIndex $selectedConfigIndex

         	# get the values for the selected configuration
         	getConfigValues
	}
}


#----------------------------------------------------------------------
# function called to obtain all values ie.initialize
#----------------------------------------------------------------------
proc getAllRobotValues {} {

	global NumRobots RobotNameList QueryFileName
	##puts "get Robot Values entered"

	# if window is not open yet these does not exist
	if { [ winfo exists .genQueryDialog] == 0 } {
		return
	}

   	#-----------------------------
	# initialize default values
	#-----------------------------
	set NumRobots 0
   	set QueryFileName [QueryGetFileName]

	# get the number of robots currently available
	set NumRobots [QueryGetNumberRobots]

	# delete all the current values
	if { [llength $RobotNameList] != 0} {
		$RobotNameList delete 0 $NumRobots
	}

	if { $NumRobots > 0 } {

		# get the current list of robot names to be displayed
		for {set i 0} {$i < $NumRobots} {incr i} {
			set robotName [ QueryGetRobotName $i ]
			$RobotNameList insert end $robotName
      		}

		# highlight any current selection if any
		set selectedRobotIndex [ QueryGetPickedRobotIndex ]
		$RobotNameList activate $selectedRobotIndex
		$RobotNameList selection set $selectedRobotIndex $selectedRobotIndex
      		getRobotConfigs
	}
}
#----------------------------------------------------------------------
# function called when an actor's current configuration is saved as goal
#----------------------------------------------------------------------
proc getCurrentPosition {} {
	#puts stdout "getCurrentPosition"
	# get the currently selected robot's current position/orientation in space
   	set result [QueryGetCurrentConfiguration]
   	# refresh display
   	getConfigValues
}

#----------------------------------------------------------------------
# function called when an actor is selected in the listbox
#----------------------------------------------------------------------
proc RobotSelected {} {
	global NumRobots RobotNameList
	set selected [$RobotNameList curselection]
	# should only be one element in this list if single selection
	set index [ lindex $selected 0 ]
	set result [ QuerySetPickedRobotIndex $index ]
	# capture focus
	set old [focus]
	# refresh displayed values
	set result [getRobotConfigs]
	return $result
	# return focus to this listbox
	focus $old
}

#----------------------------------------------------------------------
# function called when a goal position is selected in the listbox
#----------------------------------------------------------------------
proc GoalSelected {} {
	global NumRobotConfigs RobotConfigList
	set selected [$RobotConfigList curselection]
	# should only be one element in this list if single selection
	set index [ lindex $selected 0 ]
	set result [ QuerySetPickedConfigIndex $index ]
	# refresh displayed values
	set result [getConfigValues]
	return $result
}

#----------------------------------------------------------------------
# functions called when Save button is pressed
#----------------------------------------------------------------------
proc OnSaveQuery {w} {
	global SUCCESS FAILURE QueryFileName QueryFileTypes mainwindow
	global EnvironmentWorkingDir FALSE CANCEL NO_WRITE_PERMISSION
	#puts "OnSaveQuery entered"
   	# all values were already written to query, only save it now
   	set result $SUCCESS
	set QueryFileName [QueryGetFileName]
	set filepath $QueryFileName
	set filename [file tail $filepath]
	if {  [ string compare $filename "Untitled.cfg"] == 0 } {
   		set typeList $QueryFileTypes
		set filepath [tk_getSaveFile -filetypes $typeList -parent $mainwindow \
			-initialfile $QueryFileName -initialdir $EnvironmentWorkingDir \
			-defaultextension ".cfg" -title "Save Query As"]
		if { [ file exists filepath] == $FALSE } {
			set pathTail [file tail $filepath] 
			set filepath "$EnvironmentWorkingDir/$pathTail"
		}
		set filename [file tail $filepath]
	}
   	if { [string compare $filename ""] != 0} {
		set fileDir [file dirname $filepath] 
		if { [ file writable $fileDir] == 0 } {
			SendUserMessage $NO_WRITE_PERMISSION
			return $CANCEL
		}
		set result [ QuerySave $filepath ]
      		set result [ QueryClose ]
      		destroy $w
	}
   	return $result
}
#----------------------------------------------------------------------
# functions called when Close button is pressed
#----------------------------------------------------------------------
proc OnCloseQuery {w} {
	# don't update the values only close the window and destroy the query
   	set result [QueryClose]
	destroy $w
   	return $result
}


