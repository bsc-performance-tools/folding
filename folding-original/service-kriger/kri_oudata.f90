subroutine kri_oudata
  !-----------------------------------------------------------------------
  !****f* kri_oudata
  ! NAME
  !    kri_oudata
  ! DESCRIPTION
  !	- In the first section, the output files (ascii) are created. One general
  !	  and one for each design variables. 
  !
  !	- In the second section, a Gnuplot sript file is built. The file contains
  !	  the script to produced a multi-window plot (one plot for each design variable
  !	  set as a constant. Each plot contains one curve for each values of this constant design variable).
  !	  After, an example of how to creat a postscript fil of one plot is given.
  !	  NOTE: This step is performed only if the number of design variable is lower than 3.        
  !-----------------------------------------------------------------------

  use def_kriger
  implicit none

  integer(ip)      :: i,j,idesi,iwrit	!count
  integer(ip)      :: ikrig,isamp,nsadi,vdesi,jchek,numfi
  integer(1)	   :: dvcst,dvar1,dvar2

  real(rp)         :: tolcg,reste,obmin,obmax,check,origx,origy
  character*200    :: woout,woaux,word


! 1 *** Main file with all the data 

  write (lun_oudat_kri,*)  '# Kriger output'
  write (lun_oudat_kri,*)  '# -------------'
  write (lun_oudat_kri,*)  '#   Number of samples : ',nsamp_kri
  write (lun_oudat_kri,*)  '#   Sampled design variables: ',ndesi_kri, ' first columns'  
  write (lun_oudat_kri,*)  '#   Sampled objectives: the last columns'  
  do isamp= 1,nsamp_kri
     write (lun_oudat_kri,100)  (xsamp_kri(vdesi,isamp),vdesi=1,ndesi_kri),xobje_kri(1,isamp)
  end do
  write (lun_oudat_kri,*)  
  write (lun_oudat_kri,*)  
  write (lun_oudat_kri,*)  '#   Number of kriged points : ',nkrig_kri
  write (lun_oudat_kri,*)  '#   Kriged design variables: ',ndesi_kri, ' first columns'  
  write (lun_oudat_kri,*)  '#   Kriged objectives: the last columns'  
  do ikrig= 1,nkrig_kri
     write (lun_oudat_kri,100)  (kriso_kri(vdesi,ikrig),vdesi=1,ndesi_kri),kriso_kri(ndesi_kri+1,ikrig)
  end do



! 2 *** GNUPLOT OUTPUT DATA FILES 

if (mgrid_kri .EQ. 0) then  !Don't create any other output if no grid is used
	goto 999
end if 

if (ndesi_kri .EQ. 1) then

	dvar1=1 !The lowest of the two dvvar
	numfi=lun_oudv1_kri

	write (lun_outgn_kri(1),*)  "# to read, type: load 'kriger.plo'"
	write (lun_outgn_kri(1),*)  "reset;clear"
	write (lun_outgn_kri(1),*)  "set pointsize 2"
	write (lun_outgn_kri(1),*)  'set title "Interpolation"'
	write (lun_outgn_kri(1),*)  'set xlabel "',paran_kri(dvar1),'"'
	write (lun_outgn_kri(1),*)  'set ylabel "',paran_kri(mobta_kri+ndesi_kri),'"'

	!The first index is the sample data
	do i=1,nsamp_kri
		write(numfi,100) xsamp_kri(1,i),xobje_kri(1,i)
	end do
	write(numfi,*) ''
	write(numfi,*) ''


	do j=1,nkrig_kri
		write(numfi,100) (kriso_kri(iwrit,j),iwrit=1,2)
	end do

	write (lun_outgn_kri(1),*)  "plot 'krig_dv1.out' index 0 using 1:2 with points pointtype 5,\" 
	write (lun_outgn_kri(1),*)  "'krig_dv1.out' index 1 using 1:2 with lines"

end if 

!***************************

if (ndesi_kri .EQ. 2) then

	dvar1=1 !The lowest of the two dvvar
	dvar2=2
	numfi=lun_oudv1_kri

	write (lun_outgn_kri(1),*)  "# to read, type: load 'kriger.plo'"
	write (lun_outgn_kri(1),*)  "reset;clear"

	write (lun_outgn_kri(1),*)  "#set hidden3d"
	write (lun_outgn_kri(1),*)  "set nokey"
	write (lun_outgn_kri(1),*)  "set pointsize 2"
	write (lun_outgn_kri(1),*)  "set ticslevel 0"
	write (lun_outgn_kri(1),*)  "#set pm3d at b"
	write (lun_outgn_kri(1),*)  "#set pm3d"



	write (lun_outgn_kri(1),*)  " "

	write (lun_outgn_kri(1),*)  'set title "Interpolation"'
	write (lun_outgn_kri(1),*)  'set xlabel "',paran_kri(dvar1),'"'
	write (lun_outgn_kri(1),*)  'set ylabel "',paran_kri(dvar2),'"'
	write (lun_outgn_kri(1),*)  'set zlabel "',paran_kri(mobta_kri+ndesi_kri),'"'

	!The first index is the sample data
	do i=1,nsamp_kri
		write(numfi,100) (xsamp_kri(iwrit,i),iwrit=1,2),xobje_kri(1,i)
	end do
	write(numfi,*) ''
	write(numfi,*) ''

	jchek=0.0
	do j=1,nkrig_kri

		write(numfi,100) (kriso_kri(iwrit,j),iwrit=1,3)
		jchek=jchek+1
		reste=mod(jchek,nkrdi_kri(dvar1))					
		if (reste .EQ. 0.0) then
			write(numfi,*) '  '
		end if	
	end do
	write(numfi,*) ''


	write (lun_outgn_kri(1),*)  "splot 'krig_dv1.out' index 0 using 1:2:3 with points pointtype 5,\" 
	write (lun_outgn_kri(1),*)  "'krig_dv1.out' index 1 using 1:2:3 with lines"



end if 

!******

if (ndesi_kri .EQ. 3) then

	!find the min and the max of the objective variable
	obmin=xobje_kri(1,1)
	obmax=xobje_kri(1,1)
	do i=1,nsamp_kri
		if (xobje_kri(1,i) .GT. obmax) then 
			obmax=xobje_kri(1,i)
		end if
		if (xobje_kri(1,i) .LT. obmin) then 
			obmin=xobje_kri(1,i)
		end if  
	end do


	do idesi=1,3 !one file for each constant design variable

		 !----------------------------
		  write (lun_outgn_kri(idesi),*)  "# to read, type: load 'kriger.plo'"
		  write (lun_outgn_kri(idesi),*)  "reset;clear"
		  write (lun_outgn_kri(idesi),*)  "set hidden3d"
		  write (lun_outgn_kri(idesi),*)  "set nokey"
		  write (lun_outgn_kri(idesi),*)  "set pointsize 1.5"
		  write (lun_outgn_kri(idesi),*)  "set ticslevel 0"
		  write (lun_outgn_kri(idesi),*)  " "
		 !----------------------------

		if (idesi .EQ. 1) then
			dvcst=1
			dvar1=2 !The lowest of the two dvvar
			dvar2=3
			numfi=lun_oudv1_kri
			origx=0.0
			origy=0.5
		end if 

		if (idesi .EQ. 2) then
			dvcst=2
			dvar1=1 !The lowest of the two dvvar
			dvar2=3
			numfi=lun_oudv2_kri
			origx=0.5
			origy=0.5
		end if 

		if (idesi .EQ. 3) then
			dvcst=3
			dvar1=1 !The lowest of the two dvvar
			dvar2=2
			numfi=lun_oudv3_kri
			origx=0.0
			origy=0.0
		end if 

		!The first index is the sample data
		do i=1,nsamp_kri
			write(numfi,100) (xsamp_kri(iwrit,i),iwrit=1,3),xobje_kri(1,i)
		end do
		write(numfi,*) ''
		write(numfi,*) ''
		!

		jchek=0.0
		do i=1,xnbva_kri(1,idesi)

			do j=1,nkrig_kri
				!if (kriso_kri(dvcst,j) .EQ. varli_kri(dvcst,i)) then

				if (kriso_kri(dvcst,j) .EQ. varli_kri(dvcst,i)) then
					write(numfi,100) (kriso_kri(iwrit,j),iwrit=1,4)
					jchek=jchek+1
						reste=mod(jchek,nkrdi_kri(dvar1))					
						if (reste .EQ. 0.0) then
							write(numfi,*) '  '
						end if
				end if
			end do
			write(numfi,*) ''
		end do


		  ! GnuPlot SCRIPT file
		  write (lun_outgn_kri(idesi),*)  'set xrange[',dkrig_kri(dvar1,1)-(0.1*abs(dkrig_kri(dvar1,1))),&
					   ':',dkrig_kri(dvar1,2)+(0.1*abs(dkrig_kri(dvar1,2))),']'
		  write (lun_outgn_kri(idesi),*)  'set yrange[',dkrig_kri(dvar2,1)-(0.1*abs(dkrig_kri(dvar2,1))),&
					   ':',dkrig_kri(dvar2,2)+(0.1*abs(dkrig_kri(dvar2,2))),']'
		  write (lun_outgn_kri(idesi),*)  'set zrange[',obmin-(0.1*abs(obmin)),':',obmax+(0.1*abs(obmax)),']'
		  write (lun_outgn_kri(idesi),*)  'set title "',paran_kri(dvcst),' is constant"'
		  write (lun_outgn_kri(idesi),*)  'set xlabel "',paran_kri(dvar1),'"'
		  write (lun_outgn_kri(idesi),*)  'set ylabel "',paran_kri(dvar2),'"'
		  write (lun_outgn_kri(idesi),*)  'set zlabel "',paran_kri(mobta_kri+ndesi_kri),'"'
		  write (lun_outgn_kri(idesi),*)  "splot 'krig_dv1.out' index 0 using",dvar1,&
					  ":",dvar2,":4"," with points pointtype 5,\" 
		  write (lun_outgn_kri(idesi),*)  "'",fil_oudvv_kri(idesi),"' index 1:",&
					  jchek+1, "using",dvar1,":",dvar2,":4"," with lines"
	 	 write (lun_outgn_kri(idesi),*)  ' '
		
		 write (lun_outgn_kri(idesi),*)  "set nomultiplot" 

		  !An example of ps file 
		  write (lun_outgn_kri(idesi),*)  "  "
		  write (lun_outgn_kri(idesi),*)  "# Example of ps file  "
		  write (lun_outgn_kri(idesi),*)  "# Erase the comment character of the following lines "
		  write (lun_outgn_kri(idesi),*)  "# simply cut & past the sript of the graph to output,"
		  write (lun_outgn_kri(idesi),*)  "#  and repeated it for all the graphs"
		  write (lun_outgn_kri(idesi),*)  " "
		  write (lun_outgn_kri(idesi),*) '#set term post eps enh "Times-Roman" 14'
		  write (lun_outgn_kri(idesi),*) '#set size 1.0 ,1.0'
		  write (lun_outgn_kri(idesi),*)  " "
		  write (lun_outgn_kri(idesi),*) '#set output "example.eps"'
		  write (lun_outgn_kri(idesi),*) '#   copy the lines here"'


	 end do !idesi
	  




end if

!**************

if (ndesi_kri .GT. 3) then
  write(6,*) '--|    * NO plot file available:'
  write(6,*) '--|      number of design variables is greater than 3 *'
end if

  write(6,*) '--|    '
  write(6,*) '--|    Minimun objective value:',mingl_kri
  write(6,*) '--|    At point:',minpt_kri


  write(6,*) '--|    Maximum objective value:',maxgl_kri
  write(6,*) '--|    At point:',maxpt_kri
  write(6,*) '--|    '

!write(*,*) 'maxgl_kri ',maxgl_kri
!write(*,*) 'maxpt_kri ',maxpt_kri



100 format (30(2x,E10.5))

999 continue 

end subroutine kri_oudata
