# Thank you: https://memorynotfound.com/using-gnuplot-to-plot-apache-benchmark-data/
# output as png image
set terminal png size 1024,768

# save file to "top-results.png"
set output "top-results.png"

# graph title
set title "Yimmo Resource Utilization"

#nicer aspect ratio for image size
#set size 1,0.7

# y-axis grid
#set grid y

#x-axis label
set xlabel "seconds"

#y-axis label
set ylabel "% CPU"

set yrange [0:200]
set ytics 25 nomirror tc lt 1

set y2range [0:100]
set y2label "Mem (Mb)"
set y2tics 10 nomirror tc lt 2

set datafile separator comma
set grid

plot "top.data" using 0:1 smooth sbezier with lines title "%CPU" lw 2 axis x1y1, '' using 0:2 smooth sbezier with lines title "Mem (Mb)" lw 2 axis x1y2
