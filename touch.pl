#!/usr/bin/perl

$t_log_files = "LOGFILES";

system("find . | grep cpp > $t_log_files");

open FILES, "<$t_log_files" or die "can't open $t_log_files\n";

while(<FILES>) {

  chomp;
  $file = $_;
  $newfile = $file;
  $newfile =~ s/.cpp/.d/;
  $cmd = "touch $newfile";
  print "cmd: $cmd \n";
  system("$cmd");

}

system("touch ./src/gui/moc_main_win.d");
system("touch ./src/gui/moc_scene_win.d");
system("touch ./src/gui/moc_animation_gui.d");
system("touch ./src/gui/moc_snapshot_gui.d");
system("touch ./src/gui/moc_itemselection_gui.d");
system("touch ./src/gui/moc_roadmap.d");
system("touch ./src/gui/moc_filelistDialog.d");
system("touch ./src/gui/moc_obj_property.d");
system("touch ./src/gui/moc_obprmGUI.d");
system("touch ./src/gui/moc_addObjDialog.d");
system("touch ./src/gui/moc_vizmoEditor.d");
system("touch ./src/gui/moc_queryGUI.d");
