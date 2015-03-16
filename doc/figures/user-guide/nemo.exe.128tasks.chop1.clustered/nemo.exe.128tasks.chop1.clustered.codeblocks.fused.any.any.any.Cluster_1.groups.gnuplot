# set term postscript eps enhaced solid color
# set term png size 800,600
set term wxt size 800,600;

set datafile separator ";"
set key bottom outside center horizontal samplen 1;
set yrange [-1:1];
set noytics; 
set xtics nomirror rotate by -90; 
set border 1
set xlabel "Time (ms)";
set title "Instance groups for Appl * Task * Thread * - Region Cluster_1\n1261 Instances - No instance grouping"

set style line 1 pt 1 lc rgb "#FF0000"
set style line 2 pt 2 lc rgb "#00B000"
set style line 3 pt 3 lc rgb "#0000FF"
set style line 4 pt 4 lc rgb "#FF8000"
set style line 5 pt 5 lc rgb "#FF0080"
set style line 6 pt 6 lc rgb "#00A0A0"

set style line 7 pt 1 lc rgb "#FF8080"
set style line 8 pt 2 lc rgb "#80B080"
set style line 9 pt 3 lc rgb "#8080FF"
set style line 10 pt 4 lc rgb "#FF8080"
set style line 11 pt 5 lc rgb "#FF8080"
set style line 12 pt 6 lc rgb "#80A0A0"

set label "" at 155773783.0/1000000.0,0 point lt 1 ps 2 lc rgb "#FF0000";

plot \
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.groups.csv' using (strcol(1) eq 'u' && (strcol(2) eq 'Cluster_1' && $3 == 1) ? $4 / 1000000.0: NaN) : $0 ti 'Group_0 (803/458)' w points ls 1,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.groups.csv' using ((strcol(1) eq 'e') && (strcol(2) eq 'Cluster_1' && $3 == 1) ? $4 / 1000000.0: NaN) : $0 notitle w points ls 7;

unset label;
unset arrow;
