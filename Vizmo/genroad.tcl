#script to create/handle the Generate RoadMap dialog box

#foreach {i j} { Environment Untitled.env RoadMap Untitled.map } {
#	set genRoadNameVal($i) $j
#	puts stdout $genRoadNameVal($i)
#}

set rmdoneVar 0


#----------------------------------------------------------------------
# create the  Generate RoadMap dialog box
#----------------------------------------------------------------------
proc GenerateRoadmapDialog {w} {
	global genRoadNameVal
        
         ShowParameters $w roadmap
	#set result [ getGenRoadValues ]

}

#----------------------------------------------------------------------
# main routine to create and display the Generate RoadMap  dialog 
# called from main window
#----------------------------------------------------------------------

proc displayGenerateRoadMapDialog {} {
    global rmdoneVar
    global mainwindow
    set parent $mainwindow
    grab $parent
    set w .genRoadmapDialog
    catch {destroy $w}
    toplevel $w
    wm title $w "Generate Roadmap"
    wm iconname $w "Generate Roadmap"
    wm geometry $w +100+100
    ShowParameters $w roadmap 
    grab release $parent
}


#------------------------------------------------------------------------
# call functions to get values to be displayed 
#------------------------------------------------------------------------
proc getGenRoadValues {} {

	global genRoadNameVal

	# if window is not open yet these do not exist
	if { [ winfo exists .genRoadmapDialog] == 0 } {
		#puts stdout "genRoadmapDialog window unopen"
		return 
	}
	foreach {i j} { Environment 0 RoadMap 1} {
		set genRoadNameVal($i) [GetFileNames $j]
	}
}

#------------------------------------------------------------------------
# call functions to get new files if required
#------------------------------------------------------------------------

proc SelectRoadOptions {type w} {
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
		 -title "Select $type" -initialdir $thisDir -defaultextension $thisExt]

	} elseif { $type == "RoadMap" } {
		set typeList $RoadMapFileTypes
		set thisDir $EnvironmentWorkingDir
		set thisExt ".map"
		set filepath [tk_getSaveFile -filetypes $typeList -parent $mainwindow \
		 -title "Select $type" -initialfile $defaultMapFile -initialdir $thisDir -defaultextension $thisExt]

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

proc genRoadMapOnStart {w} { 
	global rmdoneVar genRoadNameVal FAILURE
	# need to call the function to start the roadmap generation
	foreach i { Environment RoadMap} {
		if {[string compare $genRoadNameVal($i) "" ] == 0 } {
			set result $FAILURE
			return $result
		}
	}
	set rmdoneVar 1
	destroy $w
	set result [RoadMapGenerate ]
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

proc RoadMapGenerate {w commandLine params} {
	global genRoadNameVal MakemapExeName IsEnvironmentModifiedFlag TRUE SUCCESS
	global DataFilesDir mainwindow EnvironmentWorkingDir FAILURE 
	global READY NORMAL WAIT GENERATING_MAP_FILE CANCEL NO_WRITE_PERMISSION
  global exe
  global env_file
  global env_dir
  global map_name
  global p
  global q
  global genRoadNameVal

  global astar
  global rs
  global rs_value
  global sl
  global e
  global se
  global se_value
  global nodesperobst
  global nshells
  global nseeds
  global nodes
  global seeds_option1
  global seeds_option2
  global free_option1
  global free_option2
  global num_free
  global gnodes
  global cnodes
  global dm
  global cd
  global misc_params


	# check if user specified an env or mod file
	set genRoadEnv $genRoadNameVal(Environment)
			if { $IsEnvironmentModifiedFlag == $TRUE} {
				set choice [tk_messageBox -title "Vizmo 3D" \
      				-parent $mainwindow -type yesnocancel -default yes \
      				-message "Save Current Environment?" \
         			-icon question ]
				if { [string compare $choice "yes" ] == 0 } {
					set result [ EnvironmentSave $genRoadEnv $modPath $iniPath]
					if {$result == $SUCCESS } {
   						set IsEnvironmentModifiedFlag 0
        					set EnvironmentFileName [EnvironmentGetFileName]
   					} else {
      						# reset flags
      						set EnvironmentFileName [EnvironmentGetFileName]
   					}
				} elseif { [string compare $choice "cancel" ] == 0 } {
					return $CANCEL
				}
			}
	set genRoadMapfile $genRoadNameVal(RoadMap)
	#set commandLine $MakemapExeName
        #set commandLine "/pub/dsmft/burc/vizmo/a"
        #puts "Command Line= $commandLine"
	# should ensure change to same directory as map so intermed files can be stored
	catch {	cd [file dirname $genRoadMapfile]  cdError }
	set result $SUCCESS
	UpdateStatus $GENERATING_MAP_FILE
	UpdateCursor $WAIT
	#remove error log if it exists
	file delete ./RoadmapError.txt
         #aptal(1)= "-f" 
         #aptal(2)= "2cubes.env" 
         #puts "Args=$params"
	if [catch {  eval exec  $commandLine    $params\
                 >& ./RoadmapError.txt  } result]  {
			UpdateStatus $READY
			UpdateCursor $NORMAL	
			#puts stdout $result
                        
			set choice [tk_messageBox -title "Vizmo 3D - Generate Roadmap Failure" \
      				-parent $mainwindow -type ok -default ok \
      				-message "$result" \
         			-icon error ]
	}
	UpdateStatus $READY
	UpdateCursor $NORMAL
	if { [file exists $genRoadMapfile] == 1 } {
		set choice [tk_messageBox -title "Vizmo 3D- RoadMap Generation Successful" \
      				-parent $mainwindow -type ok -default ok \
      				-message "$genRoadMapfile successfully created." \
	         			-icon info ]
	} else {
		set choice [tk_messageBox -title "Vizmo 3D - RoadMap Generation Failure" \
      			-parent $mainwindow -type ok -default ok \
      			-message "Check the Error Log ./RoadmapError.txt" \
      	   		-icon error ]
	}
  	destroy $w
	return $result
}
	
