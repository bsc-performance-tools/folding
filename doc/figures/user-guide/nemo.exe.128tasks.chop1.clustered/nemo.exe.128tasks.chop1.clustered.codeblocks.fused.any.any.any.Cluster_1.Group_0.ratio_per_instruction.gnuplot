X_LIMIT=155.72 # Do not touch this
FACTOR=1 # Do not touch this

set datafile separator ";";

# set term postscript eps solid color;
# set term pdfcairo solid color lw 2;
# set term png size 800,600;
set term wxt size 800,600;

set multiplot title "Appl * Task * Thread * - Group_0 - Cluster_1"

##############################
## Routines part 
##############################

samplecls(ret,r,g,t) = (r eq 'Cluster_1' && g == 0  && t eq 'cl') ? ret : NaN;

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


set obj rect from graph 0.000*FACTOR, graph 0 to graph 0.050*FACTOR, graph 1 fs transparent solid 0.33 noborder fc rgbcolor '#ff0000' behind # Routine: stp 4.985%
set obj rect from graph 0.050*FACTOR, graph 0 to graph 0.553*FACTOR, graph 1 fs transparent solid 0.33 noborder fc rgbcolor '#00ff00' behind # Routine: tra_ldf_iso 50.267%
set obj rect from graph 0.574*FACTOR, graph 0 to graph 0.921*FACTOR, graph 1 fs transparent solid 0.33 noborder fc rgbcolor '#0000ff' behind # Routine: tra_zdf_imp 34.715%
set obj rect from graph 0.928*FACTOR, graph 0 to graph 0.995*FACTOR, graph 1 fs transparent solid 0.33 noborder fc rgbcolor '#ffa000' behind # Routine: eos_insitu_pot 6.786%

# Summary for routine eos_insitu_pot 6.786%
# Summary for routine nemo 0.004%
# Summary for routine nemo_gcm 0.009%
# Summary for routine stp 8.209%
# Summary for routine tra_ldf 0.001%
# Summary for routine tra_ldf_iso 50.267%
# Summary for routine tra_zdf 0.010%
# Summary for routine tra_zdf_imp 34.715%

set label center "nemo >\nnemo_gcm >\nstp\n[208]" at second 0.025*FACTOR, graph 0.5 rotate by 90 tc rgbcolor 'black' front
set label center "stp >\ntra_ldf >\ntra_ldf_iso\n[212]" at second 0.301*FACTOR, graph 0.5 rotate by 90 tc rgbcolor 'black' front
set label center "stp >\ntra_zdf >\ntra_zdf_imp\n[204]" at second 0.747*FACTOR, graph 0.5 rotate by 90 tc rgbcolor 'black' front
set label center "nemo_gcm >\nstp >\neos_insitu_pot\n[352]" at second 0.961*FACTOR, graph 0.5 rotate by 90 tc rgbcolor 'black' front


plot "nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.dump.csv" u ($4*FACTOR):(samplecls($5,strcol(2),$3,strcol(1))) with points axes x2y2 ti '' lc rgbcolor '#ff2090' pt 7 ps 0.5;

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

slope_PAPI_BR_CN(ret,c,r,g) = (c eq 'PAPI_BR_CN' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_BR_CN(ret,c,r,g) = (c eq 'PAPI_BR_CN_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_BR_MSP(ret,c,r,g) = (c eq 'PAPI_BR_MSP' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_BR_MSP(ret,c,r,g) = (c eq 'PAPI_BR_MSP_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_BR_UCN(ret,c,r,g) = (c eq 'PAPI_BR_UCN' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_BR_UCN(ret,c,r,g) = (c eq 'PAPI_BR_UCN_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_FP_INS(ret,c,r,g) = (c eq 'PAPI_FP_INS' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_FP_INS(ret,c,r,g) = (c eq 'PAPI_FP_INS_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_L1_DCM(ret,c,r,g) = (c eq 'PAPI_L1_DCM' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_L1_DCM(ret,c,r,g) = (c eq 'PAPI_L1_DCM_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_L2_DCM(ret,c,r,g) = (c eq 'PAPI_L2_DCM' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_L2_DCM(ret,c,r,g) = (c eq 'PAPI_L2_DCM_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_L3_TCM(ret,c,r,g) = (c eq 'PAPI_L3_TCM' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_L3_TCM(ret,c,r,g) = (c eq 'PAPI_L3_TCM_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_LD_INS(ret,c,r,g) = (c eq 'PAPI_LD_INS' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_LD_INS(ret,c,r,g) = (c eq 'PAPI_LD_INS_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_SR_INS(ret,c,r,g) = (c eq 'PAPI_SR_INS' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_SR_INS(ret,c,r,g) = (c eq 'PAPI_SR_INS_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_TOT_CYC(ret,c,r,g) = (c eq 'PAPI_TOT_CYC' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_TOT_CYC(ret,c,r,g) = (c eq 'PAPI_TOT_CYC_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_TOT_INS(ret,c,r,g) = (c eq 'PAPI_TOT_INS' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_VEC_DP(ret,c,r,g) = (c eq 'PAPI_VEC_DP' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_VEC_DP(ret,c,r,g) = (c eq 'PAPI_VEC_DP_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_PAPI_VEC_SP(ret,c,r,g) = (c eq 'PAPI_VEC_SP' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_PAPI_VEC_SP(ret,c,r,g) = (c eq 'PAPI_VEC_SP_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_RESOURCE_STALLS(ret,c,r,g) = (c eq 'RESOURCE_STALLS' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_RESOURCE_STALLS(ret,c,r,g) = (c eq 'RESOURCE_STALLS_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_RESOURCE_STALLS_LB(ret,c,r,g) = (c eq 'RESOURCE_STALLS:LB' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_RESOURCE_STALLS_LB(ret,c,r,g) = (c eq 'RESOURCE_STALLS:LB_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_RESOURCE_STALLS_ROB(ret,c,r,g) = (c eq 'RESOURCE_STALLS:ROB' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_RESOURCE_STALLS_ROB(ret,c,r,g) = (c eq 'RESOURCE_STALLS:ROB_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_RESOURCE_STALLS_RS(ret,c,r,g) = (c eq 'RESOURCE_STALLS:RS' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_RESOURCE_STALLS_RS(ret,c,r,g) = (c eq 'RESOURCE_STALLS:RS_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_RESOURCE_STALLS_SB(ret,c,r,g) = (c eq 'RESOURCE_STALLS:SB' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
ratio_RESOURCE_STALLS_SB(ret,c,r,g) = (c eq 'RESOURCE_STALLS:SB_per_ins' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;

plot \
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_BR_CN($5, strcol(3), strcol(1), $2)) ti 'PAPI_BR_CN/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_BR_MSP($5, strcol(3), strcol(1), $2)) ti 'PAPI_BR_MSP/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_BR_UCN($5, strcol(3), strcol(1), $2)) ti 'PAPI_BR_UCN/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_FP_INS($5, strcol(3), strcol(1), $2)) ti 'PAPI_FP_INS/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_L1_DCM($5, strcol(3), strcol(1), $2)) ti 'PAPI_L1_DCM/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_L2_DCM($5, strcol(3), strcol(1), $2)) ti 'PAPI_L2_DCM/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_L3_TCM($5, strcol(3), strcol(1), $2)) ti 'PAPI_L3_TCM/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_LD_INS($5, strcol(3), strcol(1), $2)) ti 'PAPI_LD_INS/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_SR_INS($5, strcol(3), strcol(1), $2)) ti 'PAPI_SR_INS/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_TOT_CYC($5, strcol(3), strcol(1), $2)) ti 'PAPI_TOT_CYC/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(slope_PAPI_TOT_INS($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x2y2 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_VEC_DP($5, strcol(3), strcol(1), $2)) ti 'PAPI_VEC_DP/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_PAPI_VEC_SP($5, strcol(3), strcol(1), $2)) ti 'PAPI_VEC_SP/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_RESOURCE_STALLS($5, strcol(3), strcol(1), $2)) ti 'RESOURCE_STALLS/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_RESOURCE_STALLS_LB($5, strcol(3), strcol(1), $2)) ti 'RESOURCE_STALLS:LB/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_RESOURCE_STALLS_ROB($5, strcol(3), strcol(1), $2)) ti 'RESOURCE_STALLS:ROB/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_RESOURCE_STALLS_RS($5, strcol(3), strcol(1), $2)) ti 'RESOURCE_STALLS:RS/ins' axes x2y1 w lines lw 3,\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4*FACTOR):(ratio_RESOURCE_STALLS_SB($5, strcol(3), strcol(1), $2)) ti 'RESOURCE_STALLS:SB/ins' axes x2y1 w lines lw 3;

unset label;
unset arrow;

unset multiplot;
