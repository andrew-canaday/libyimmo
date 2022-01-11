# Thank you: https://memorynotfound.com/using-gnuplot-to-plot-apache-benchmark-data/
# output as png image
set terminal png

# save file to "ab-results.png"
set output "ab-results.png"

# graph title
set title "Yimmo Apache Bench Results"

#nicer aspect ratio for image size
#set size 1,0.7

# y-axis grid
set grid y

#x-axis label
set xlabel "request"

#y-axis label
set ylabel "response time (ms)"

plot "ab.data" using 9 smooth sbezier with lines title "Yimmo Response Times"
