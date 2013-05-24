module def_kriger
  !-----------------------------------------------------------------------
  !****f* def_kriger
  ! NAME
  !    def_kriger
  ! DESCRIPTION
  !    Heading for kriger
  !***
  !-----------------------------------------------------------------------

  use def_kinrea
  implicit none

  integer(ip)         ::&
	lun_indat_kri, lun_oudat_kri,&  !unit number for the ext. files (kri_openfi.f90)
	lun_oudv1_kri,lun_oudv2_kri,lun_oudv3_kri,larpl_kri
	
  integer(ip),dimension(3) ::lun_outgn_kri

  integer(ip)         :: &
	nembe_kri,ndesi_kri,nobje_kri,nsamp_kri,nkrig_kri,noffs_kri,&
	nmatr_kri,mobta_kri,mobje_kri,nsolv_kri,mgrid_kri,drift_kri,lntot_kri  			!(kri_indata.f90)

  integer(ip)         :: &
       kfl_covar_kri         ! covariance type

  integer(ip),pointer :: &
       nkrdi_kri(:)          ! kriging mesh definition # of intervals (ndesi_kri)

  real(rp),pointer    :: &
       xsamp_kri(:,:),&      ! samples design values (ndesi_kri x nsamp_kri)
       xobje_kri(:,:),&      ! samples objective values (nobje_kri x nsamp_kri)
       xkrig_kri(:,:),&      ! kriged nodes design values  (ndesi_kri x nkrig_kri)
       xkrob_kri(:,:),&      ! kriged nodes objective values  (nobje_kri x nkrig_kri)
       xmaco_kri(:,:),&      ! Covariance matrix column (nsamp_kri x nsamp_kri)
       dkrig_kri(:,:),&      ! kriging mesh definition paramenters (2 , ndesi_kri)
       xmaca_kri(:,:),&      ! Augmented Covariance matrice 
       order_kri(:,:),&      ! Order of the permuted line fot the LU solver   
       xnbva_kri(:,:),&      ! Le nombre de valeurs de chaques variable de design
       varli_kri(:,:),&	     ! Les valeurs de chaques variable de design  
       minpt_kri(:),&
       maxpt_kri(:),& 		 
       kriso_kri(:,:)	     !  Solution kriging matrice => The Answer

  character(6),dimension(:),pointer  :: paran_kri !Names of the design and obj variables 	

 character(12),dimension(3) :: fil_oudvv_kri	!Names of the output files - One file for each design 

  real(rp)            :: &
       rcova_kri(3),mingl_kri,maxgl_kri,nuget_kri            ! covariance definition parameters


contains
  
  !------------------------------------------
  !
  ! decoding function
  !
  !------------------------------------------
  subroutine decodi(worin,worou)
    implicit none
    character(150) :: worin, woaux, worou(5)
    integer(8)     :: leng,i,k,inew
    worou= ' '
    woaux = ' '
    i=1
    k=1
    inew=1
    leng = 150
    do while(i<=leng) 
       if (worin(i:i)=='=' .or. worin(i:i)==':' .or. worin(i:i)==',') then
          worou(k)= adjustl(trim(woaux))
          woaux   = ' '
          inew    = i+1
          k=k+1
       else
          woaux= worin(inew:i)     
       end if
       if (i==leng) worou(k)= adjustl(trim(woaux))
       i=i+1
    end do
    
  end subroutine decodi
  
  
end module def_kriger
