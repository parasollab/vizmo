# script to create the Generate Path dialog box
# has callbacks to run the OBPRM Query program
#----------------------------------------------------------------------
# initialization of global variables - done when file is sourced
# have to be global in scope 
#----------------------------------------------------------------------

#foreach {i j} { Environment Untitled.env RoadMap Untitled.map \
#		Query Untitled.cfg Path Untitled.path } {
#	set genPathNameVal($i) $j
#	#puts stdout $genPathNameVal($i)
#}

set doneVar 0
set nTryConnect 100
#puts stdout $nTryConnect
set LocalPlanner "straightline"
#puts stdout $LocalPlanner

#----------------------------------------------------------------------
# create the Generate Path dialog box
#----------------------------------------------------------------------
proc GeneratePathDialog {w} {
	global genPathNameVal nTryConnect EnvironmentFileName
	#puts stdout "GenPathDialog"
	foreach i { Environment RoadMap Query Path } {
		frame $w.f$i -width 40 
		label $w.f$i.l$i -text "$i" -width 20
		entry $w.f$i.e$i -textvariable genPathNameVal($i) -width 40
		button $w.f$i.b$i -text Select -command "Select $i $w"
		pack $w.f$i.l$i -side left -fill y -ipadx 3 -anchor e
		pack  $w.f$i.b$i $w.f$i.e$i -side right -fill y -ipadx 3
		pack $w.f$i -side top -padx 5 -fill both -pady 5 -ipadx 3 -ipady 5
	}
	set genPathNameVal(Environment) $EnvironmentFileName
	frame $w.fConnect  -width 40
	label $w.fConnect.lConn -text "Connection Nodes" -width 20
	entry $w.fConnect.eConn -textvariable nTryConnect -width 20
	pack $w.fConnect.lConn $w.fConnect.eConn -side left -fill y -ipadx 3 -anchor e
	pack $w.fConnect -side top -padx 5 -fill both -pady 5 -ipadx 3 -ipady 5
	frame $w.buttons -bd 1 -relief groove
	pack $w.buttons -side bottom -fill x -ipady 10 

	#can only create paths for currently selected environment
	$w.fEnvironment.eEnvironment configure -state disabled
	$w.fEnvironment.bEnvironment configure -state disabled
	#------------------------------------------------------------------------
	#create the buttons on the lower panel
	#------------------------------------------------------------------------
	# send all the variables entered to the generate function
	button $w.buttons.start -text Start  -width 10 -command "genPathOnStart $w"
	button $w.buttons.cancel -text Cancel  -width 10 -command "genPathOnCancel $w"
	pack $w.buttons.start $w.buttons.cancel -side left -expand 1

	catch {tkwait visibility $w}
	catch {grab $w}

	#------------------------------------------------------------------------
	# call functions to get values to be displayed 
	#------------------------------------------------------------------------
	set result [ getGenPathValues ]
	return $result 
}

#----------------------------------------------------------------------
# main routine to create and display the GeneratePath dialog
# called from main window
#----------------------------------------------------------------------
proc displayGeneratePathDialog {} {
	global doneVar
	#puts stdout "displayGeneratePathDialog"
    	global mainwindow
    	set parent $mainwindow
	set old [focus]
    	set w .genPathDialog
    	catch {destroy $w}
    	toplevel $w
    	wm title $w "Run Query"
    	wm iconname $w "Run Query"
    	wm geometry $w +100+100
	set doneVar 0
    	ShowParameters $w query
	#tkwait variable doneVar
    	grab release $w
	focus $old
} 


#------------------------------------------------------------------------
# call functions to get values to be displayed 
#------------------------------------------------------------------------
proc getGenPathValues {} {
	global genPathNameVal nTryConnect

	# if window is not open yet these do not exist
	if { [ winfo exists .genPathDialog] == 0 } {
		##puts stdout "genPathDialog window unopen"
		return 
	}
	foreach {i j} { Environment 0 RoadMap 1 Query 2 Path 3} {
		set genPathNameVal($i) [GetFileNames $j]
	}
	set nTryConnect 100
}

#------------------------------------------------------------------------
# call functions to get new files if required
#------------------------------------------------------------------------

proc Select {type w} {
	global SUCCESS FAILURE genPathNameVal EnvironmentFileTypes RoadMapFileTypes
	global QueryFileTypes PathFileTypes mainwindow EnvironmentWorkingDir DefaultWorkingDir
	set genPathEnv $genPathNameVal(Environment)
	set fileroot [file rootname $genPathEnv]
	set defaultPathFile $EnvironmentWorkingDir/$fileroot.path
	# need to find the filename to be displayed
	#puts stdout $type 
	if { $type == "Environment" } {
		set typeList $EnvironmentFileTypes
		set thisDir $DefaultWorkingDir
		set thisExt ".env"
		set filepath [tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		 -title "Select $type" -initialdir $thisDir -defaultextension $thisExt]
	} elseif { $type == "RoadMap" } {
		set typeList $RoadMapFileTypes
		set thisDir $EnvironmentWorkingDir
		set thisExt ".map"
		set filepath [tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		 -title "Select $type" -initialdir $thisDir -defaultextension $thisExt]
	} elseif { $type == "Query" } {
		set typeList $QueryFileTypes
		set thisDir $EnvironmentWorkingDir
		set thisExt ".cfg"
		set filepath [tk_getOpenFile -filetypes $typeList -parent $mainwindow \
		 -title "Select $type" -initialdir $thisDir -defaultextension $thisExt]
	} elseif { $type == "Path" } {
		set typeList $PathFileTypes
		set thisDir $EnvironmentWorkingDir
		set thisExt ".path"
		set filepath [tk_getSaveFile -filetypes $typeList -parent $mainwindow \
		 -title "Select $type" -initialfile $defaultPathFile -initialdir $thisDir -defaultextension $thisExt]
	} else {
		return $FAILURE
	}
	set filename [file tail $filepath]
	if {[string compare $filename "" ] != 0 } {
		set genPathNameVal($type) $filepath
	}
	# this is neat way to bring your other dialog on top
	raise $w 
	
}

proc genPathOnStart {w} { 
	global doneVar genPathNameVal FAILURE nTryConnect SUCCESS mainwindow
	# need to call the function to start the query processing
	foreach i { Environment RoadMap Query Path } {
		if {[string compare $genPathNameVal($i) "" ] == 0 } {
			set result $FAILURE
			return $result
		}
	}
	if {$nTryConnect < 0} {
		set $nTryConnect 100
	}
	set result [PathGenerate $w]
	
}

proc genPathOnCancel {w} {
	global doneVar
	set doneVar 1
	destroy $w
}


proc PathGenerate {w commandLine params} {

	global genPathNameVal CANCEL FAILURE SUCCESS GENERATING_PATH_FILE 
	global READY NORMAL WAIT NO_WRITE_PERMISSION
	global nTryConnect QueryExeName CANCEL doneVar
	global LocalPlanner mainwindow EnvironmentWorkingDir
	# check if user specified an env or mod file
	set genPathEnv $genPathNameVal(Environment)
	set fileext [file extension $genPathEnv]
	set fileroot [file rootname $genPathEnv]
		
	
	set genPathMap $genPathNameVal(RoadMap)
	set genPathQuery $genPathNameVal(Query)
	set genPathPath $genPathNameVal(Path)
	#set commandLine $QueryExeName
	#set params ""
	if { [file exists $genPathMap] == 0 } {
		set result $FAILURE
		#puts stdout "invalid map file"
		return $result 
        }
        if { [file exists $genPathQuery] == 0 } {
                set result $FAILURE
                #puts stdout "invalid query file"
                return $result 
        } 




	# check that environment is not newer than roadmap
	if [newer $genPathEnv $genPathMap] {
		set choice [tk_messageBox -title "Vizmo 3D Path Generate" \
      			-parent $mainwindow -type yesno -default yes \
      			-message "Environment is newer than Map file! Continue?" \
         			-icon warning ]
		if { [string compare $choice "no"] == 0 } {
			return $CANCEL
		}
	}
 	# perform actual execution
	#puts stdout "$commandLine $params"
	# change to the current directory for intermediate files 
	#catch {cd $EnvironmentWorkingDir } cdError 
	# if user just typed a filename, path is pwd so u need previous line

	# check modification times
	set envDir [ file dirname $genPathPath]
	if { [file writable $envDir ] == 0 } {
			SendUserMessage $NO_WRITE_PERMISSION
			return $FAILURE	
	}
	# must delete the old path file
	if { [file exists $genPathPath] == 1 } {
		if [catch { file delete $genPathPath } result] {
			set choice [tk_messageBox -title "Vizmo 3D" \
      				-parent $mainwindow -type ok -default ok \
      				-message "Cannot delete existing path $genPathPath! Close path and retry." \
         				-icon error ]
			return $result 
		}
	}
	set result $SUCCESS
	# should ensure change to same directory as path so intermed files can be stored
	catch { cd [file dirname $genPathPath]  cdError }
	UpdateStatus $GENERATING_PATH_FILE
	UpdateCursor $WAIT
	#remove error log if it exists
	file delete ./QueryError.txt
	if  [ catch {eval exec $commandLine $params \
		 		>& ./QueryError.txt } errorVal ] {
		UpdateStatus $READY
		UpdateCursor $NORMAL
		set result $FAILURE
		set choice [tk_messageBox -title "Vizmo 3D - Run Query failed!" \
      			-parent $mainwindow -type ok -default ok \
      			-message "$errorVal. Check the Error Log ./QueryError.txt" \
         		-icon error ]
		set doneVar 1
		destroy $w 
	} else { 
		UpdateStatus $READY
		UpdateCursor $NORMAL
		if { [file exists $genPathPath] == 1 } {
			set choice [tk_messageBox -title "Vizmo 3D - Run Query Successful!" \
      				-parent $mainwindow -type yesno -default yes \
      				-message "Display Path $genPathPath?" \
         			-icon question ]
      		if { [string compare $choice "yes" ] == 0} {
      				set doneVar 1
					destroy $w
					set result [ DisplayPath $genPathPath]
			} else {
				set doneVar 1
				destroy $w 
			}
		} else {
			set doneVar 1
			destroy $w 
		}
	}
}

