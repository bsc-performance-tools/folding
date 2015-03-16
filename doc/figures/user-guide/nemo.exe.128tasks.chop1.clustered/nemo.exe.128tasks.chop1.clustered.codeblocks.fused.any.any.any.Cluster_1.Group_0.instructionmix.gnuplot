X_LIMIT=155.72 # Do not touch this
FACTOR=1 # Do not touch this

set datafile separator ";";

# set term postscript eps solid color;
# set term pdfcairo solid color lw 2;
# set term png size 800,600;
set term wxt size 800,600;

set multiplot title "Evolution for Instruction mix model\nAppl * Task * Thread * - Group_0 - Cluster_1"


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


set style histogram rowstacked;
set style data histogram;
set style fill solid 1 noborder;

set key bottom outside center horizontal samplen 1;
set y2range [0:*];
set yrange [0:1];
set x2range [0:1000*1./FACTOR];
set xtics nomirror format "%.02f";
unset x2tics
set xlabel 'Time (in ms)';
set ylabel 'Instruction type';
set y2label 'MIPS';
set ytics nomirror;
set y2tics nomirror format "%g";
set xtics ( 0.0 , 1./5.*X_LIMIT, 2./5.*X_LIMIT, 3./5.*X_LIMIT, 4./5.*X_LIMIT, 5./5.*X_LIMIT, X_LIMIT/FACTOR);
set xrange [0:X_LIMIT*1./FACTOR];
# Unneeded phases separators, nb. breakpoints = 2

slope_instructionmix_ld_mix(ret,c,r,g) = (c eq 'instructionmix_ld_mix' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_st_mix(ret,c,r,g) = (c eq 'instructionmix_st_mix' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_br_u_mix(ret,c,r,g) = (c eq 'instructionmix_br_u_mix' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_br_c_mix(ret,c,r,g) = (c eq 'instructionmix_br_c_mix' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_fp_mix(ret,c,r,g) = (c eq 'instructionmix_fp_mix' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_vec_spdp_mix(ret,c,r,g) = (c eq 'instructionmix_vec_spdp_mix' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_others(ret,c,r,g) = (c eq 'instructionmix_others' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;
slope_instructionmix_mips(ret,c,r,g) = (c eq 'instructionmix_mips' && r eq 'Cluster_1' && g == 0 ) ? ret : NaN;

plot \
'< grep ^"Cluster_1;0;instructionmix_ld_mix;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_ld_mix($5, strcol(3), strcol(1), $2)) ti 'LD' axes x2y1 lc rgbcolor 'red',\
'< grep ^"Cluster_1;0;instructionmix_st_mix;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_st_mix($5, strcol(3), strcol(1), $2)) ti 'ST' axes x2y1 lc rgbcolor 'blue',\
'< grep ^"Cluster_1;0;instructionmix_br_u_mix;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_br_u_mix($5, strcol(3), strcol(1), $2)) ti 'uncond BR' axes x2y1 lc rgbcolor 'green',\
'< grep ^"Cluster_1;0;instructionmix_br_c_mix;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_br_c_mix($5, strcol(3), strcol(1), $2)) ti 'cond BR' axes x2y1 lc rgbcolor 'yellow',\
'< grep ^"Cluster_1;0;instructionmix_fp_mix;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_fp_mix($5, strcol(3), strcol(1), $2)) ti 'FP' axes x2y1 lc rgbcolor 'orange',\
'< grep ^"Cluster_1;0;instructionmix_vec_spdp_mix;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_vec_spdp_mix($5, strcol(3), strcol(1), $2)) ti 'VEC sp+dp' axes x2y1 lc rgbcolor 'purple',\
'< grep ^"Cluster_1;0;instructionmix_others;" nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u (slope_instructionmix_others($5, strcol(3), strcol(1), $2)) ti 'Others' axes x2y1 lc rgbcolor '#808080',\
'nemo.exe.128tasks.chop1.clustered.codeblocks.fused.any.any.any.slope.csv' u ($4 * X_LIMIT):(slope_instructionmix_mips($5, strcol(3), strcol(1), $2)) ti 'MIPS' axes x1y2 w lines lw 3 lc rgbcolor 'black';

unset label;
unset arrow;

unset multiplot;
