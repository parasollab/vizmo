#!/bin/env perl

$output_string = $ARGV[0];
$error_file = $ARGV[1];
$expected_num_errors = $ARGV[2];

open(BUILD,"<../../../GNUmakefile.thisbuild");

while ($line = <BUILD>) {
  if ($line =~ /platform/) {
    chomp $line;
    ($trash, $platform) = split("=",$line);
  }
}

if ((!($platform =~ /gcc/)) && (!($platform =~ /xlC/))) {
  print " Don't know how to count for this compiler... ";
  $num_errors = -1;
} else {
  open(ERRORS,"<$error_file");

  while ($line = <ERRORS>) {
    if ($platform =~ /gcc/) {
      if (($line =~ /: no match/) || 
          ($line =~ /: no/) || 
          ($line =~ /: conversion/) ||
          ($line =~ /: invalid conversion/) ||
          ($line =~ / is ambiguous/) ||
          ($line =~ /: cannot convert/)) {
        $num_errors += 1;
      }
    } elsif ($platform =~ /xlC/) {
      if ($line =~ /\(S\)/) {
        $num_errors += 1;
      }
    }
  }

}

print "$output_string";
if ($expected_num_errors == $num_errors) {
  print "Passed\n";
} else {
  print "Failed... $num_errors\n";
}
