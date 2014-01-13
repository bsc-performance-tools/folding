subroutine kri_openfi(itask)
  !-----------------------------------------------------------------------
  !****f* kri_openfi
  ! NAME
  !    kri_openfi
  ! DESCRIPTION
  !    open files
  !***
  !-----------------------------------------------------------------------
  use def_kriger
  implicit none
  integer(ip), intent(in)   :: itask
  character(150)            :: fil_indat_kri,fil_oudat_kri

  character(12),dimension(3) :: fil_outgn_kri!Names of the gnuplot files - One file for each cte design para 

  select case(itask)
     
  case(1)
     lun_indat_kri = 8001
     lun_oudat_kri = 8002

     lun_oudv1_kri = 8004
     lun_oudv2_kri = 8005
     lun_oudv3_kri = 8006

     lun_outgn_kri(1) = 8010
     lun_outgn_kri(2) = 8011
     lun_outgn_kri(3) = 8012

     fil_indat_kri = 'kriger.dat'   	 ! input file
     fil_oudat_kri = 'kriger.out'   	 ! output file
     fil_oudvv_kri(1)=	'krig_dv1.out'   ! output file
     fil_oudvv_kri(2)=	'krig_dv2.out'   ! output file
     fil_oudvv_kri(3)=	'krig_dv3.out'   ! output file
    
     fil_outgn_kri(1) = 'dv1_cte.plo'   ! gnuplot batch file
     fil_outgn_kri(2) = 'dv2_cte.plo'   ! gnuplot batch file
     fil_outgn_kri(3) = 'dv3_cte.plo'   ! gnuplot batch file

     open(lun_indat_kri,file=fil_indat_kri,status='old'    )
     open(lun_oudat_kri,file=fil_oudat_kri,status='unknown')
     open(lun_oudv1_kri,file=fil_oudvv_kri(1),status='unknown')
     open(lun_oudv2_kri,file=fil_oudvv_kri(2),status='unknown')
     open(lun_oudv3_kri,file=fil_oudvv_kri(3),status='unknown')
     



     open(lun_outgn_kri(1),file=fil_outgn_kri(1),status='unknown')
     open(lun_outgn_kri(2),file=fil_outgn_kri(2),status='unknown')
     open(lun_outgn_kri(3),file=fil_outgn_kri(3),status='unknown')

  case(2)
     close(lun_indat_kri)
     close(lun_oudat_kri)  
     close(lun_oudv1_kri) 
     close(lun_oudv2_kri)        
     close(lun_oudv3_kri)               
     close(lun_outgn_kri(1))     
     close(lun_outgn_kri(2))     
     close(lun_outgn_kri(3))     
  end select


end subroutine kri_openfi
