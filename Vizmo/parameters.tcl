
proc ShowParameters {w typ} {
#frame $w

frame $w.exe
 global env_name
  global env_name2
  global env_file
  global map_name
  global EnvironmentFileName MapFileName

  global p
  global q

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


 set env_name [file rootname $EnvironmentFileName ]
 set env_name2 [file rootname $EnvironmentFileName ]
 set map_name $MapFileName 
 ##puts "mapname = $MapFileName"
 Update $w
 

set exe "/pub/dsmft/GMS-OBPRM/BASELINE"
label $w.exe.exe_label -text "Executable"
entry $w.exe.exe_entry -width 50 -textvariable exe
$w.exe.exe_entry delete 0 end
$w.exe.exe_entry insert 0 $exe
pack $w.exe.exe_label $w.exe.exe_entry -anchor w -side left -padx 5 -pady 5
pack $w.exe -side top -padx 5 -pady 5

frame $w.env1

set env_dir [pwd]
##puts "env=$env_dir"
label $w.env1.env_dir_label -text "Environment Dir"
entry $w.env1.env_dir_entry -width 40 -textvariable env_dir 
$w.env1.env_dir_entry delete 0 end
$w.env1.env_dir_entry insert 0 $env_dir
pack $w.env1.env_dir_label $w.env1.env_dir_entry -anchor w -side left -padx 5 -pady 5

##puts "here1"
label $w.env1.env_name_label -text "Environment Name"
##puts "here6"

#puts "w=$w"
entry $w.env1.env_name_entry -width 20 -textvariable env_name2
 #$w.env1.env_name_entry  insert 0 $env_name
#puts "here7"
pack $w.env1.env_name_label $w.env1.env_name_entry -anchor w -side left -padx 5 -pady 5
#puts "here5"

button $w.env1.update -text Update -command "Update $w"

pack $w.env1.update -side left -pady 2

pack $w.env1 -anchor w -side top -padx 5 -pady 5

#puts "here3"
frame $w.env2

#puts "here9"
set env_file $env_name
#puts "here12"
append env_file ".env"
#puts "here10"
label $w.env2.env_label -text "Environment File"
entry $w.env2.env_entry -width 20 -textvariable env_file
#puts "here11"

pack $w.env2.env_label $w.env2.env_entry -anchor w -side left -padx 5 -pady 5

set p $env_name
#append map_name ".map"
label $w.env2.map_label -text "Map File"
entry $w.env2.map_entry -width 30 -textvariable map_name
pack $w.env2.map_label $w.env2.map_entry -anchor w -side left -padx 5 -pady 5

pack $w.env2 -anchor w -side top -padx 5 -pady 5

frame $w.env3
set q $env_name
append q ".query"
label $w.env3.q_label -text "Query File"
entry $w.env3.q_entry -width 30 -textvariable q
pack $w.env3.q_label $w.env3.q_entry -anchor w -side left -padx 5 -pady 5

pack $w.env3 -anchor w -side top -padx 5 -pady 5


set p $env_name
append p ".path"
label $w.env3.p_label -text "Path File"
entry $w.env3.p_entry -width 30 -textvariable p
pack $w.env3.p_label $w.env3.p_entry -anchor w -side left -padx 5 -pady 5

frame $w.set1

frame $w.set1.lp

label $w.set1.lp.lp_text -text "Local Planners"
listbox $w.set1.lp.lp_lb
pack $w.set1.lp.lp_text $w.set1.lp.lp_lb -side top 
pack $w.set1.lp -side left 

foreach i {straightline rotate_at_s astar_clearance astar_distance} {
  $w.set1.lp.lp_lb insert end $i
}

frame $w.set1.b
button $w.set1.b.b1 -text "-> LPSet 0" -command "add_lp $w $w.set1.lp0.lp_set0"
button $w.set1.b.b2 -text "-> LPSet 1" -command "add_lp $w $w.set1.lp1.lp_set1"
button $w.set1.b.b3 -text "-> LPSet 2" -command "add_lp $w $w.set1.lp2.lp_set2"

pack $w.set1.b.b1 $w.set1.b.b2 $w.set1.b.b3 -side top
pack $w.set1.b -side left

frame $w.set1.lp0
label $w.set1.lp0.lp0-text -text "LP Set 0"
listbox $w.set1.lp0.lp_set0
pack $w.set1.lp0.lp0-text $w.set1.lp0.lp_set0 -side top
pack $w.set1.lp0 -side left

frame $w.set1.lp1
label $w.set1.lp1.lp1-text -text "LP Set 1"
listbox $w.set1.lp1.lp_set1
pack $w.set1.lp1.lp1-text $w.set1.lp1.lp_set1 -side top
pack $w.set1.lp1 -side left

frame $w.set1.lp2
label $w.set1.lp2.lp2-text -text "LP Set 2"
listbox $w.set1.lp2.lp_set2
pack $w.set1.lp2.lp2-text $w.set1.lp2.lp_set2 -side top
pack $w.set1.lp2 -side left

frame $w.set1.dm

label $w.set1.dm.label -text "Distance Metrics"
pack $w.set1.dm.label -side top
set dist_metrics "Euclid Scaled_Euclid"
set b 0
foreach item $dist_metrics {
  frame $w.set1.dm.$b
  radiobutton $w.set1.dm.$b.$b -variable dm -text $item -value $item
  pack $w.set1.dm.$b.$b -side left
  if { $item == "Scaled_Euclid"} {
    label $w.set1.dm.$b.label$b -text "S="
    set se 0.5
    entry $w.set1.dm.$b.entry$b -width 5 -textvariable se
    pack $w.set1.dm.$b.entry$b $w.set1.dm.$b.entry$b -side left 
  }
  pack $w.set1.dm.$b -side top -anchor w
  incr b
}

label $w.set1.dm.cd_label -text "Collision Detection"
pack $w.set1.dm.cd_label -side top -pady 10
tk_optionMenu $w.set1.dm.cd_menu cd cstk vclip
pack $w.set1.dm.cd_menu -side top

pack $w.set1.lp $w.set1.dm -side left -pady 2

pack $w.set1 -anchor w -side top -padx 5 -pady 5


frame $w.set2

set nodes 10
tk_optionMenu $w.set2.num_nodes_type nodesPerObst nodes nodesPerObst 
pack $w.set2.num_nodes_type -side left   -padx 5 -pady 5


#label $w.set2.nodes_label -text "nodes"
entry $w.set2.nodes_entry -width 10 -textvariable nodes
#pack $w.set2.nodes_label $w.set2.nodes_entry -anchor w -side left -padx 5 -pady 5
pack $w.set2.nodes_entry -anchor w -side left -padx 5 -pady 5

#set nseeds 50
#label $w.set2.nseeds_label -text "nseeds"
#entry $w.set2.nseeds_entry -width 10 -textvariable nseeds
#pack $w.set2.nseeds_label $w.set2.nseeds_entry -anchor w -side left -padx 5 -pady 5

set nshells 3 
label $w.set2.nshells_label -text "nshells"
entry $w.set2.nshells_entry -width 10 -textvariable nshells
pack $w.set2.nshells_label $w.set2.nshells_entry -anchor w -side left -padx 5 -pady 5
$w.set2.nshells_entry configure -state disabled

#set nodesperobst 10
#label $w.set2.nodesperobst_label -text "nodesPerObst"
#entry $w.set2.nodesperobst_entry -width 10 -textvariable nodesperobst
#pack $w.set2.nodesperobst_label $w.set2.nodesperobst_entry -anchor w -side left -padx 5 -pady 5

label $w.set2.seeds_option_label -text "Collision pair"
tk_optionMenu $w.set2.seeds_option_menu1 seeds_option1 cM rV rT rE rW cM_rV rV_rT rV_rW all
tk_optionMenu $w.set2.seeds_option_menu2 seeds_option2 cM rV rT rE rW cM_rV rV_rT rV_rW all
pack $w.set2.seeds_option_label $w.set2.seeds_option_menu1 $w.set2.seeds_option_menu2 -side left

pack $w.set2 -anchor w -side top -padx 5 -pady 5

frame $w.set3

set num_free 5
label $w.set3.num_free_label -text "Edges "
entry $w.set3.num_free_entry -width 10 -textvariable num_free
pack $w.set3.num_free_label $w.set3.num_free_entry -anchor w -side left -padx 5 -pady 5

label $w.set3.free_option_label -text "Free Pair"
tk_optionMenu $w.set3.free_option_menu1 free_option1  cM rV rT rE rW cM_rV rV_rT rV_rW all
tk_optionMenu $w.set3.free_option_menu2 free_option2  cM rV rT rE rW cM_rV rV_rT rV_rW all
pack $w.set3.free_option_label $w.set3.free_option_menu1 $w.set3.free_option_menu2  -side left

label $w.set3.gnodes_label -text "gnodes"
tk_optionMenu $w.set3.gnodes_menu gnodes BasicPRM BasicOBPRM OBPRM
pack $w.set3.gnodes_label $w.set3.gnodes_menu -side left

bind $w.set3.gnodes_menu <Configure> [ gnodes_bind $w ]

label $w.set3.cnodes_label -text "cnodes"
tk_optionMenu $w.set3.cnodes_menu cnodes closest
pack $w.set3.cnodes_label $w.set3.cnodes_menu -side left

pack $w.set3 -anchor w -side top -padx 5 -pady 5

frame $w.set4
label $w.set4.misc_label -text "Additional Command Line Paramaters"
entry $w.set4.misc_entry -textvariable misc_params -width 50
pack $w.set4.misc_label $w.set4.misc_entry -side left
pack $w.set4 -anchor w -side top -padx 5 -pady 5

frame $w.b
button $w.b.run_mkmp -text "Makemap" -command "Run $w  obprm"
button $w.b.run_query -text "Query" -command "Run $w query"
button $w.b.cl_file -text "Save to File" -command "CL_File $w"
button $w.b.quit -text Quit -command "destroy $w"
pack $w.b.run_mkmp $w.b.run_query $w.b.quit -side left
pack $w.b -side bottom -pady 2
 bind $w.set1.lp0.lp_set0 <ButtonRelease-1> \
   {ListDeleteSel %W %y}

 bind $w.set1.lp1.lp_set1 <ButtonRelease-1> \
   {ListDeleteSel %W %y}

 bind $w.set1.lp2.lp_set2 <ButtonRelease-1> \
   {ListDeleteSel %W %y}


#pack $w
  if { [string compare $typ "roadmap"] == 0 } {
  $w.b.run_query  configure -state disabled
   
 }
  if { [string compare $typ "query"] == 0 } {
  $w.b.run_mkmp  configure -state disabled
  
 }

}
proc CL_File {w} {
  toplevel .file
  label .file.filename_label -text "Filename:"
  entry .file.filename_entry -textvariable filename
  pack .label.filename_label .file.filename_entry -side left

  button .file.ok -text "OK" -command "Save"
  button .file.cancel -text "Cancel -command "destroy .file"
  pack .file.ok .file.cancel -side left
}

proc Save {} {

}

proc Run {w f} {
  global exe
  global env_file
  global env_dir
  global map_name
  global p
  global q
  global genRoadNameVal
  global genPathNameVal

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
  global params

  set params ""

  append params " -f "
  append params [file rootname $env_file ]

#  set env_name [file rootname $EnvironmentFileName ]
  #puts "envfile= $env_file"

  append params " -envFile "
  append params $env_file

  #append params " -outmapFile "
  #append params $map_name

  set count 0
  foreach item [$w.set1.lp0.lp_set0 get 0 end] {
    if {$count == 0} {
      append params " -lp"
      set count 1
    }
    append params " "
    append params $item
  }

  set count 0
  foreach item [$w.set1.lp1.lp_set1 get 0 end] {
    if {$count == 0} {
      append params " -lp"
      set count 1
    }
    append params " "
    append params $item
  }

  set count 0
  foreach item [$w.set1.lp2.lp_set2 get 0 end] {
    if {$count == 0} {
      append params " -lp"
      set count 1
    }
    append params " "
    append params $item
  }
 
  set dms ""

  if {$dm == "Euclid"} {
    append dms " euclidean"
  }

  if {$dm == "Scaled_Euclid"} {
    append dms " scaledEuclidean "
    append dms $se
  }

  if [string compare $dms ""] {
    append params " -dm"
    append params $dms
  }

  append params " -cd "
  append params $cd

  append params " -nodes "
  append params $nodes

  #append params " -$nodesperobst"

  #append params $nodes

  append params " -nshells "
  append params $nshells

  #append params " -nodesPerObst "
  #append params $nodesperobst

  append params " -collPair "
  append params $seeds_option1 " " $seeds_option2

  append params " -edges "
  append params $num_free

  append params " -freePair "
  append params $free_option1 " " $free_option2

  append params " -gNodes "
  append params $gnodes

  append params " -cNodes "
  append params $cnodes

  if { [string compare $f "query"] == 0 } {
     append params " -inmapFile "
     append params $map_name

     append params " -queryFile "
     append params $q

     append params " -pathFile "
     append params $p


  }
  if { [string compare $f "obprm"] == 0 } {
     append params " -outmapFile "
     append params $map_name


  }



  if [string compare $misc_params ""] {
    append params $misc_params
  } 

  set cl $exe
  append cl "/"
  append cl $f
  append cl " "
  append cl $params

  #puts stdout "cd $env_dir"
  #puts stdout "$cl"

  set tmp $env_file
  #append tmp ".env" 

   if { [string compare $f "obprm"] == 0 } {
     set genRoadNameVal(Environment)  $tmp
     set genRoadNameVal(RoadMap)  $map_name

     set cl $exe
     append cl "/"
     append cl $f
      RoadMapGenerate $w $cl $params
  }
   if { [string compare $f "query"] == 0 } {
       set genPathNameVal(Environment)  $tmp
       set genPathNameVal(RoadMap)  $map_name
       set genPathNameVal(Query) $q
       set genPathNameVal(Path) $p
       PathGenerate $w $cl $params
   }

}

proc Update {w} {
  global env_name
  global env_name2
  global q
  global p
  global env_file
  global map_name
  
  #puts "here"

  #set env_name $env_name2
  set q $env_name2
  append q ".query"

  set p $env_name2
  append p ".path"

  set env_file $env_name2
  append env_file ".env"

  #set map_name $env_name2
  #append map_name ".map"
  #puts "leaving"

}


proc add_lp {w a} {
  set i [$w.set1.lp.lp_lb get active]
  if {$i == "straightline"} {
     add_sl_to_list $a $i
  }

  if {$i == "rotate_at_s"} {
    toplevel .rs
    append rs 1
    frame .rs.1
    label .rs.1.label_s -text "S = "
    entry .rs.1.get_s -width 5 -textvariable rs
    pack .rs.1.label_s .rs.1.get_s -side left
    pack .rs.1 -side top

    frame .rs.button
    button .rs.button.ok -text OK -command "add_rs_to_list $a $i"
    button .rs.button.cancel -text "Cancel" -command "destroy .rs"
    pack .rs.button.ok .rs.button.cancel -side left
    pack .rs.button -side top
  }

  if {$i == "astar_clearance"} {
    toplevel .astar
    set as1 6
    set as2 3 
    frame .astar.1
    label .astar.1.label1 -text "Number Tries"
    entry .astar.1.get1 -width 5 -textvariable as1
    pack .astar.1.label1 .astar.1.get1 -side left
    pack .astar.1 -side top -anchor e

    frame .astar.2
    label .astar.2.label2 -text "Number Neighbors"
    entry .astar.2.get2 -width 5 -textvariable as2
    pack .astar.2.label2 .astar.2.get2 -side left
    pack .astar.2 -side top -anchor e

    frame .astar.button
    button .astar.button.ok -text OK -command "add_astar_to_list $a $i"
    button .astar.button.cancel -text "Cancel" -command "destroy .astar"
    pack .astar.button.ok .astar.button.cancel -side left
    pack .astar.button -side top
  }

  if {$i == "astar_distance"} {
    toplevel .astar
    set as1 6
    set as2 3
    entry .astar.get1 -width 5 -textvariable as1
    entry .astar.get2 -width 5 -textvariable as2

    button .astar.button -text OK -command "add_astar_to_list $w $i"
    pack .astar.get1 .astar.get2 .astar.button
  }

}

proc add_sl_to_list {w i} {
  $w insert end $i
}

proc add_rs_to_list {w i} {
  global rs

  set value "$i $rs"
  $w insert end $value
  destroy .rs
}

proc add_astar_to_list {w i} {
  global as1
  global as2

  $w insert end "$i $as1 $as2"
  destroy .astar
}

 proc ListDeleteSel {w y} {
  foreach i [lsort -integer -decreasing [$w curselection]] {
     $w delete $i
  }
}

proc gnodes_bind {w} {
  global gnodes

  if { $gnodes == "PRM" } {
    $w.set2.nshells_entry configure -state disabled -bg "dark gray"
  }

  if { $gnodes == "OBPRM" } {
    $w.set2.nshells_entry configure -state normal -bg white
  }
}


