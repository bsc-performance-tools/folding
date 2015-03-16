X_LIMIT=14647.68 # Do not touch this
FACTOR=1 # Do not touch this

set datafile separator ";";

# set term postscript eps solid color;
 set term pdfcairo solid color lw 2 font "BitStream Charter"; 
# set term png size 800,600;
#set term wxt size 800,600;

set multiplot title "Appl * Task * Thread * - Group_0 - main"

##############################
## Routines part 
##############################

samplecls(ret,r,g,t) = (r eq 'main' && g == 0  && t eq 'cl') ? ret : NaN;

set size 1,0.20;
set origin 0,0.75;

set bmargin 0; set lmargin 14; set rmargin 17;

set xlabel "ghost" tc rgbcolor "white";
set ylabel "ghost" tc rgbcolor "white";
#set y2label "Code line";
set label 'Code line' at screen 0.975, screen 0.8+(0.175/2) rotate by -90 center;
set label "bottom" at second 1.005, first 0;
set label "top"    at second 1.005, first 1;
set xrange [0:X_LIMIT*1./FACTOR];
set x2range [0:1];
set yrange [0:1];
set y2range [0:*] reverse;
set ytics tc rgbcolor "white" (0.001) format "%0.2f";
set y2tics 100 tc rgbcolor "white" format "0000";
unset xtics;
unset x2tics;


set obj rect from graph 0.000*FACTOR, graph 0 to graph 1.000*FACTOR, graph 1 fs transparent solid 0.33 noborder fc rgbcolor '#ff0000' behind # Routine: ComputeNonbondedUtil::calc_self_energy 99.993%

# Summary for routine ComputeNonbondedUtil::calc_self_energy 99.993%

set label center "ComputeNonbondedUtil::calc_self_energy\n[76]" at second 0.500*FACTOR, graph 0.5 rotate by 90 tc rgbcolor 'black' front


plot "444.namd.codeblocks.fused.any.any.any.dump.csv" u ($4*FACTOR):(samplecls($5,strcol(2),$3,strcol(1))) with points axes x2y2 ti '' lc rgbcolor '#ff2090' pt 7 ps 0.5;

unset label; unset xlabel; unset x2label; unset ylabel; unset y2label;
unset xtics; unset x2tics; unset ytics; unset y2tics; set y2tics autofreq;
unset xrange; unset x2range; unset yrange; unset y2range;
unset tmargin; unset bmargin; unset lmargin; unset rmargin
unset label;
unset arrow;
unset obj;

set size 1,0.73;
set origin 0,0;
set tmargin 0; set lmargin 14; set rmargin 17;

set key bottom outside center horizontal samplen 1;
set x2range [0:1];
set yrange [0:*];
set y2range [0:*];
#set x2tics nomirror format "%.02f";
set xlabel 'Time (in ms)';
set xtics nomirror format "%.02f";
set xtics ( 0.0 , 1./5.*X_LIMIT, 2./5.*X_LIMIT, 3./5.*X_LIMIT, 4./5.*X_LIMIT, 5./5.*X_LIMIT, X_LIMIT/FACTOR);
set xrange [0:X_LIMIT*1./FACTOR];
set ylabel 'Counter ratio per instruction';
set y2label 'MIPS';
set ytics nomirror format "%g";
set y2tics nomirror format "%g";

# Breakpoints
# Unneeded phases separators, nb. breakpoints = 2

slope_LLC_MISSES(ret,c,r,g) = (c eq 'LLC_MISSES' && r eq 'main' && g == 0 ) ? ret : NaN;
ratio_LLC_MISSES(ret,c,r,g) = (c eq 'LLC_MISSES_per_ins' && r eq 'main' && g == 0 ) ? ret : NaN;
slope_PAPI_BR_INS(ret,c,r,g) = (c eq 'PAPI_BR_INS' && r eq 'main' && g == 0 ) ? ret : NaN;
ratio_PAPI_BR_INS(ret,c,r,g) = (c eq 'PAPI_BR_INS_per_ins' && r eq 'main' && g == 0 ) ? ret : NaN;
slope_PAPI_L1_DCM(ret,c,r,g) = (c eq 'PAPI_L1_DCM' && r eq 'main' && g == 0 ) ? ret : NaN;
ratio_PAPI_L1_DCM(ret,c,r,g) = (c eq 'PAPI_L1_DCM_per_ins' && r eq 'main' && g == 0 ) ? ret : NaN;
slope_PAPI_L2_TCM(ret,c,r,g) = (c eq 'PAPI_L2_TCM' && r eq 'main' && g == 0 ) ? ret : NaN;
ratio_PAPI_L2_TCM(ret,c,r,g) = (c eq 'PAPI_L2_TCM_per_ins' && r eq 'main' && g == 0 ) ? ret : NaN;
slope_PAPI_TOT_CYC(ret,c,r,g) = (c eq 'PAPI_TOT_CYC' && r eq 'main' && g == 0 ) ? ret : NaN;
ratio_PAPI_TOT_CYC(ret,c,r,g) = (c eq 'PAPI_TOT_CYC_per_ins' && r eq 'main' && g == 0 ) ? ret : NaN;
slope_PAPI_TOT_INS(ret,c,r,g) = (c eq 'PAPI_TOT_INS' && r eq 'main' && g == 0 ) ? ret : NaN;

plot \
'444.namd.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_LLC_MISSES($5, strcol(3), strcol(1), $2)) ti 'LLC_MISSES/ins' axes x2y1 w lines lw 3,\
'444.namd.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_BR_INS($5, strcol(3), strcol(1), $2)) ti 'PAPI_BR_INS/ins' axes x2y1 w lines lw 3,\
'444.namd.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_L1_DCM($5, strcol(3), strcol(1), $2)) ti 'PAPI_L1_DCM/ins' axes x2y1 w lines lw 3,\
'444.namd.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_L2_TCM($5, strcol(3), strcol(1), $2)) ti 'PAPI_L2_TCM/ins' axes x2y1 w lines lw 3,\
'444.namd.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_TOT_CYC($5, strcol(3), strcol(1), $2)) ti 'PAPI_TOT_CYC/ins' axes x2y1 w lines lw 3,\
'444.namd.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(slope_PAPI_TOT_INS($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y2 w lines lw 3;

unset label;
unset arrow;

unset multiplot;
