#script to create/handle the Capture Video dialog box

#-------------------------------------------------------------------
# create the Capture Video dialog box
#-------------------------------------------------------------------
proc CaptureVideoDialog {w stepSize StartingFrame EndingFrame} {
	frame $w.video -height 100 -width 300 -borderwidth 2

	frame $w.video.setup -height 50 -width 50
		frame $w.video.setup.stepsize -height 20 -width 50
			label $w.video.setup.stepsize.txt -text "        Step Size"
			entry $w.video.setup.stepsize.entry -relief sunken \
			    -justify right
		pack $w.video.setup.stepsize.txt $w.video.setup.stepsize.entry \
		    -side left
		pack $w.video.setup.stepsize
		frame $w.video.setup.begin -height 20 -width 50
			label $w.video.setup.begin.txt -text "Starting Frame"
			entry $w.video.setup.begin.entry -relief sunken \
			    -justify right
		pack $w.video.setup.begin.txt $w.video.setup.begin.entry \
		    -side left
		pack $w.video.setup.begin
		frame $w.video.setup.end -height 20 -width 50
			label $w.video.setup.end.txt -text "  Ending Frame"
			entry $w.video.setup.end.entry -relief sunken \
			    -justify right
		pack $w.video.setup.end.txt $w.video.setup.end.entry -side left
		pack $w.video.setup.end
	pack $w.video.setup.stepsize $w.video.setup.begin $w.video.setup.end \
	    -side top
	pack $w.video.setup -side top
	frame $w.video.vbuttons -borderwidth 2
		button $w.video.vbuttons.okay -text "Ok" \
		    -command "okaybutton $w $stepSize $StartingFrame $EndingFrame"
		button $w.video.vbuttons.cancel -text "Cancel" -command "cancelbutton $w"
		pack $w.video.vbuttons.okay $w.video.vbuttons.cancel -side left
	pack $w.video.vbuttons -side left

	pack $w.video.setup $w.video.vbuttons -side top
	pack $w.video -side top
}

#-------------------------------------------------------------------
# main routine to create a display the Capture Video dialog box
# called from main window
#-------------------------------------------------------------------
proc displayCaptureVideoDialog {stepSize StartingFrame EndingFrame} {
	global mainwindow

	set parent $mainwindow
	grab $parent
	set w .captureVideo
	toplevel $w
	wm title $w "Capture Video"
	wm geometry $w +500+400
	focus $w
	set result [CaptureVideoDialog $w $stepSize $StartingFrame $EndingFrame]
	grab release $parent
}

#------------------------------------------------------------------
# procedure for okaybutton
#------------------------------------------------------------------
proc okaybutton {w stepSize StartingFrame EndingFrame} {
	global CameraShotTypes CameraDir
	global SAVING_FILE WAIT FAILURE NO_WRITE_PERMISSION READY NORMAL
	global mainwindow RenWin

	set stepSize [$w.video.setup.stepsize.entry get]
	set StartingFrame [$w.video.setup.begin.entry get]
	set EndingFrame [$w.video.setup.end.entry get]
	destroy	$w
	set typeList $CameraShotTypes
	set filepath [tk_getSaveFile -filetypes $typeList \
		-parent $mainwindow -initialdir $CameraDir \
		-defaultextension ".ppm" -title "Save Video Clip" ]
	puts stdout "save as $filepath"
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
	
		set CurrentFrame [PathGetCurrentFrame]
		set result [PathGoToFrame $StartingFrame]
		set numOfFrames $EndingFrame
		UpdateUI
		for {set i $StartingFrame} {$i < $numOfFrames} {incr i $stepSize} {
			set videopath [file rootname $filepath]
			if { $numOfFrames > 999 } {
				if { $i < 1000 } {
					append videopath 0
					if { $i < 100 } {
						append videopath 0
						if { $i < 10 } {
							append videopath 0
						}
					}
				}
			} elseif { $numOfFrames > 99 } {
				if { $i < 100 } {
					append videopath 0
					if { $i < 10 } {
						append videopath 0
					}
				}
			} elseif { $numOfFrames > 9} {
				if { $i < 10 } {
					append videopath 0
				}
			}
			append videopath $i
			append videopath [file extension $filepath]
			set result [$RenWin SetFileName $videopath]
			set result [$RenWin SaveImageAsPPM]
			set result [PathForwardStep]
		}
		#set videopath [file rootname $filepath]
		#append videopath $numOfFrames
		#append videopath [file extension $filepath]
		#set result [PathGotoEnd]
		#UpdateUI
		#set result [$RenWin SetFileName $videopath]
		#set result [$RenWin Save ImageAsPPM]
		set result [PathGoToFrame $StartingFrame]
		#puts stdout "result of save is $result"
		UpdateStatus $READY
		UpdateCursor $NORMAL
	}
	UpdateUI
}

#------------------------------------------------------------------
# procedure for cancelbutton
#------------------------------------------------------------------
proc cancelbutton {w} {
	destroy $w
	UpdateUI
}
