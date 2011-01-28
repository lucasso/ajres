set output 'data/leastSquares_percentError.png'
set terminal png
set xlabel "num of last measurements used in LS"
set ylabel "error in percent"
plot "data/leastSquares_percentError.txt" using 1:2 title 'percent error in least squares method'

set output 'data/leastSquares_msecError.png'
set ylabel "error in sec"
plot "data/leastSquares_percentError.txt" using 1:3 title 'msec error in least squares method'

set output 'data/bestArchitectureSearch_percentError.png'
set ylabel "error in percent"
set xlabel "total num of nrons in the net"
plot "data/bestArchitectureSearch.txt" using 1:2 title 'mean percent error while searching best arch'

set output 'data/bestArchitectureSearch_msecError.png'
set ylabel "error in msec"
plot "data/bestArchitectureSearch.txt" using 1:3 title 'mean percent error while searching best arch'
