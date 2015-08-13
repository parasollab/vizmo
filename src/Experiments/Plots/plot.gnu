#
# Stacked histograms by percent
#
set terminal postscript enhanced color
set size 1.0,0.5
set autoscale y
set border -1
set style data histograms
set style histogram gap 2
set style fill solid 0.75 border -1
set style line 1 lt 1 lw 1
set boxwidth 0.75
set key outside right samplen 1 width -2
set logscale y  
#
set output "Nodes.eps"
set ylabel "Average Nodes"
set yrange [1:12000]
set ytics 100
plot 'Oracle.Nodes.dat' using 2:xtic(1) t "PRM" ls 1 lc rgb "#E60000", \
	'' using 4:xtic(1) t "Cfg" ls 1 lc rgb "#00FF00", \
	'' using 6:xtic(1) t "Region" ls 1 lc rgb "#FFFF00", \
	'' using 8:xtic(1) t "Path" ls 1 lc rgb "#0066FF"
#

set output "CD.eps"
set ylabel "Average CD Calls"
set yrange [1:270000]
set ytics 1000
#
plot 'Oracle.CD.dat' using 2:xtic(1) t "PRM" ls 1 lc rgb "#FF0000", \
	'' using 4:xtic(1) t "Cfg" ls 1 lc rgb "#00FF00", \
	'' using 6:xtic(1) t "Region" ls 1 lc rgb "#0000FF", \
	'' using 8:xtic(1) t "Path" ls 1 lc rgb "#FF00FF"
#

set output "Time.eps"
set ylabel "Average Time"
set yrange [0.001:1500]
set ytics 100
#
plot 'Oracle.Time.dat' using 2:xtic(1) t "PRM" ls 1 lc rgb "#FF0000", \
	'' using 4:xtic(1) t "Cfg" ls 1 lc rgb "#00FF00", \
	'' using 6:xtic(1) t "Region" ls 1 lc rgb "#0000FF", \
	'' using 8:xtic(1) t "Path" ls 1 lc rgb "#FF00FF"
#
