#!/usr/local/bin/wish -f

set nodesize 10
set linewidth 3
set netborder 0.1
set maxnodex -1
set maxnodey -1
set minnodex 100000000
set minnodey 100000000
set nnodes 0
set nlinks 0
set runmode "go"
set interreport 100.0
set shownode {}

# vars
set sendcmds {}

# hack
if { $argv == "lata.net" } { set nodesize 4 }

proc readfile {} {
   global nodex nodey maxnodex maxnodey minnodex minnodey
   global linkto linkfrom nnodes nlinks netborder
   global argc argv filename

   
   set fnindex [lsearch [split $argv] "graphname"]
   if { $fnindex == "-1" } {
	puts "No graphname argument given on command line: $argv"
	exit
   }
   if { $fnindex == $argc-1 } {
	puts "No name given after graphname argument in command line: $argv"
	exit
   }
   set filename [lindex [split $argv] [expr $fnindex+1]]
   if { [file exists $filename] == "0" } {
	puts "Given graphname doesn't exist in: $argv"
	exit
   }
   set file [open $filename r]

#   puts stdout "Reading policy..."
   while {![eof $file]} {
	gets $file line
	set line [split $line]
	set type [lindex $line 0]

	# a node
	if { "$type" == 1000 } {
	   set n [lindex $line 1]
	   set x [lindex $line 2]
	   set y [lindex $line 3]
	   # puts stdout "node $n"
	   set nodex($n) $x
	   set nodey($n) $y
	   if {$x<$minnodex} {set minnodex $x}
	   if {$y<$minnodey} {set minnodey $y}
	   if {$x>$maxnodex} {set maxnodex $x}
	   if {$y>$maxnodey} {set maxnodey $y}
	   incr nnodes
	}

	# a link
	if { "$type" == 2000 } {
	   # puts stdout "link from [lindex $line 1] to [lindex $line 2]"
	   set linkfrom($nlinks) [lindex $line 1]
	   set linkto($nlinks) [lindex $line 2]
	   incr nlinks
	}
   }
   close $file

   # make sure there's some room around the edges
   set minnodex [expr $minnodex-$netborder*($maxnodex-$minnodex)]
   set maxnodex [expr $maxnodex+$netborder*($maxnodex-$minnodex)]
   set minnodey [expr $minnodey-$netborder*($maxnodey-$minnodey)]
   set maxnodey [expr $maxnodey+$netborder*($maxnodey-$minnodey)]
}

proc makedisplay {} {
   global screenwidth screenheight
   global nodesize linewidth nodex nodey maxnodex maxnodey minnodex minnodey
   global nnodes nlinks linkto linkfrom
   global pair2id
   global sim filename

   frame .panel
   canvas .net -background white -width 700 -height 500
   button .panel.quit -text "Quit" -command "exit" -relief raised
   button .panel.cont -text "Pause" -command {runtoggle} -relief raised
   button .panel.clear -text "Clear node" -command {set shownode {}; \
	.net itemconfigure link -arrow none -fill blue; \
	.net itemconfigure off -arrow none -fill yellow; \
	.net itemconfigure node -fill red } -relief raised 
#   button .panel.print -text "Print" -command ".net postscript -file $filename.ps" -relief raised
#   button .panel.redraw -text "Clear" -command "undraw; unclick" -relief raised
#   button .panel.psd -text "PSD" -command "clickpsd" -relief raised
#   checkbutton .panel.paths -text "Count Paths" -variable paths_on -relief raised
   label .panel.title -text "Interactive Router ($filename)"
   wm title . "router ($filename)"
   frame .loadctl
   scale .loadctl.loadbar -relief raised -orient horizontal -from 1 -length 250 \
	-label "Load*10" -to 40 -tickinterval 0 -command { setcmd "callmean" 0.1 }
   .loadctl.loadbar set 10
   scale .loadctl.alpha -relief raised -orient horizontal -length 250 \
	-label "Alpha*100"  -from 0 -to 100 -tickinterval 0 -command { setcmd "eta" 0.01 }
   .loadctl.alpha set 20
#   label .loadctl.title -text "Load"
   pack append .panel .panel.title left .panel.quit right .panel.cont right .panel.clear right
#   pack .panel.loadbar -side top -fill x
#   pack append .panel .panel.title left .panel.redraw left \
#	.panel.print left .panel.psd left .panel.quit right
#   pack append .panel .panel.quit bottom
#   pack append . .net bottom .panel top
   pack .panel -side top
   pack .loadctl.loadbar -side right
   pack .loadctl.alpha -side left
#   pack .loadctl.title -side left
   pack .loadctl -side top
   pack .net -side bottom

   set screenwidth [lindex [.net config -width] 4]
   set screenheight [lindex [.net config -height] 4]

   # Draw the links.
   for { set i 0 } { $i < $nlinks } { incr i } {
	set from $linkfrom($i)
	set to   $linkto($i)
	set id [.net create line [x $nodex($from) 0] \
				 [y $nodey($from) 0] \
				 [x $nodex($to)   0] \
				 [y $nodey($to)   0] \
				 -fill blue -width $linewidth \
				 -arrow none]
	.net addtag "link" withtag $id
	.net addtag "lf$i" withtag $id
	set pair2id($from,$to) $id
        .net bind $id <Button-1> "linkclick $i $id "
   }
   # Draw the nodes.
   for { set i 0 } { $i < $nnodes } { incr i } {
	set id [.net create oval [x $nodex($i) -$nodesize] \
				 [y $nodey($i) -$nodesize] \
				 [x $nodex($i)  $nodesize] \
				 [y $nodey($i)  $nodesize] \
				 -fill red -outline blue]
        .net bind $id <Button-1> \
  ".net itemconfigure node -fill red; set shownode $i; .net itemconfigure $id -fill seagreen"
	.net addtag "node" withtag $id
#	.net addtag "n$i" withtag $id
#	set id [.net create text [x $nodex($i) 0] [y $nodey($i) 0] \
#				 -fill black -text "tuna"]
#	.net addtag "t$i" withtag $id
   }
  setup_plot
}

proc x {xval add} {
   global screenwidth maxnodex minnodex
   return [expr ($xval-$minnodex)/($maxnodex-$minnodex)*$screenwidth+$add]
}
proc y {yval add} {
   global screenheight maxnodey minnodey
   return [expr $screenheight-(($yval-$minnodey)/($maxnodey-$minnodey)*$screenheight+$add)]
}

proc runtoggle {} {
   global runmode sim

   if { $runmode == "go" } {
	set runmode "paused"
	.panel.cont configure -text "Go"
   } else {
	set runmode "go"
	.panel.cont configure -text "Pause"
	.net itemconfigure link -fill blue -arrow none
	.net itemconfigure off -fill yellow -arrow none
	do_some $sim
   }
}

proc setcmd { var scale val } {
   global sendcmds
   append sendcmds " $var [expr $scale*$val]"
}

proc teardown {} {
   destroy .net
   destroy .panel
   destroy .loadctl
   destroy .plot
}

proc linkclick {i id} {
   global linkfrom linkto sendcmds
   set now [lindex [.net itemconfigure lf$i -fill] 4]
   set state [lsearch [.net gettags $id] off]
   if { $state != -1 } {
      .net dtag $id off
      .net itemconfigure "lf$i" -fill blue
      append sendcmds " up $i"
    } {
      .net addtag off withtag $id
      .net itemconfigure "lf$i" -fill yellow
      append sendcmds " down $i"
    }
}

proc startsim {} {
   global argv interreport
   set pipe "|./netsim1 interreport $interreport $argv"
# puts "p:$pipe"
   open $pipe  RDWR
}

# puts $sim {} ; flush $sim; puts [gets $sim]
proc do_some {sim} {
   global sendcmds runmode shownode

   while { $runmode == "go" } {
	# set load10 [.loadctl.loadbar get]
	# update
	# set load  [expr $load10/10.0]
	# set alpha [expr [.loadctl.alpha get]/100.0]
# puts "s:$sendcmds"
	puts $sim "$sendcmds"
	set sendcmds {}
	flush $sim
	update
	set outline [gets $sim]
	set activity [lindex $outline 1]
	set report [lindex $outline 0]
# puts "o:$outline"
# puts "a$activity"
# puts "r$report"
# puts "x[lindex $report 1]"
# puts "h[lindex $report 2]"
	plot_data [lindex $report 1] [lindex $report 2]
# puts "$load $activity"
	showlinks $activity
#	update
	if { $shownode != "" } { polclick $shownode }
   }
}

proc startup { } {
   global sim
   set sim [startsim]
   do_some $sim
}

# sizes is a list, one per link
proc showlinks {sizes} {
  global nlinks linewidth

  set scale 0.30
  for {set i 0} {$i < $nlinks} {incr i} {
     set size [expr [lindex $sizes $i]*$scale+$linewidth]
     .net itemconfigure lf$i -width $size
   }
}

# Some timeseries plots would be nice
set plot_points 40
set plot_width 800
set plot_prev [expr $plot_width-$plot_width/$plot_points]
set plot_range 20.0

proc setup_plot {} {
   global plot_points curpoint plot_width plot_prev plot_range
   toplevel .plot
   canvas .plot.wind -background white -width $plot_width -height 200
   pack .plot.wind -side left
   for { set curpoint 0 } { $curpoint < $plot_points } {incr curpoint} {
	scroll_plot
	# xmit time
	set id [.plot.wind create line $plot_prev 200 \
				 $plot_width 200 -fill blue -width 2]
	.plot.wind addtag "x$curpoint" withtag $id
	.plot.wind addtag "lines" withtag $id
	# hops
	set id [.plot.wind create line $plot_prev 200 $plot_width 200 \
				 -fill red -width 2]
	.plot.wind addtag "h$curpoint" withtag $id
	.plot.wind addtag "lines" withtag $id
   }
   incr curpoint -1

   # set up ticks
   for {set i 0} { $i<$plot_range } {incr i 5} {
	set height [expr 200-200*$i/$plot_range]
	.plot.wind create line [expr $plot_width-30] $height \
				[expr $plot_width-20] $height \
				 -fill green -width 1
   }
}

proc scroll_plot {} {
   global plot_prev plot_width
   .plot.wind move "lines" [expr $plot_prev-$plot_width] 0
}

proc plot_data {xmit hops} {
   global curpoint plot_points plot_prev plot_width plot_range

   # scale xmit and hops
   set xmit [expr 200-200*$xmit/$plot_range]
   set hops [expr 200-200*$hops/$plot_range]
   # shift counter
   set oldpoint $curpoint
   incr curpoint
   if {$curpoint == $plot_points} {set curpoint 0}

   # shift and lookup old stuff
   scroll_plot
   set oldxmit [.plot.wind coords "x$oldpoint"]
   set oldhops [.plot.wind coords "h$oldpoint"]

   # new segment
   .plot.wind coords "x$curpoint" $plot_prev [lindex $oldxmit 3] $plot_width $xmit
   .plot.wind coords "h$curpoint" $plot_prev [lindex $oldhops 3] $plot_width $hops
}

# set up policy stuff
proc polclick {dest} {
   global sim runmode
#   if { $runmode == "paused" } {
	puts $sim "pol $dest"
	flush $sim
	set outline [gets $sim]
# puts "o:[lindex $outline 0]"
	drawpol [lindex $outline 0] $dest
#   }
}

# Given a neighbor list, put arrow heads on
proc drawpol {nbrs node} {
  global nlinks nnodes linewidth pair2id
  set shape { 20 20 4 }

  # need to set links and arrows to some neutral form.
  .net itemconfigure "link" -fill blue -arrow none
  .net itemconfigure "off" -fill yellow -arrow none

  # loop through nodes.
  for {set i 0} {$i < $nnodes} {incr i} {
     if { $i != $node} {
	set nbr [lindex $nbrs $i]
	set dir [info exist pair2id($i,$nbr)]
	if { $dir == "1" } {
	   set id $pair2id($i,$nbr)
	   set col [lindex [.net itemconfigure $id -fill] 4]
	   if { $col == "blue"} { set col green }
	   .net itemconfigure $id -fill $col -arrow last -arrowshape $shape
	} else {
	   set id $pair2id($nbr,$i)
	   set col [lindex [.net itemconfigure $id -fill] 4]
	   if { $col == "blue"} { set col green }
	   .net itemconfigure $id -fill $col -arrow first -arrowshape $shape
	}
     }
  }
}


# main!!!
proc main {} {
   readfile
   makedisplay
   startup
   # do_some $sim to keep things moving!
}
main

