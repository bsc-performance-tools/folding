subroutine kri_indata(itask)
  !-----------------------------------------------------------------------
  !****f* kri_indata
  ! NAME
  !    kri_indata
  ! DESCRIPTION
  !    read input data
  !***
  !-----------------------------------------------------------------------
  use def_kriger
  implicit none
  integer(ip)      :: itask,iread,isamp,idesi,iembe,nauxi,iobje,iname,itota,iplot,i
  character(150)   :: worea,words(5),wolab,woaux
  real(rp)         :: xauxi(50)

  write (*,*) "itask=", itask

  select case(itask)
     
  case(1) ! Block 'parameters'
     
     ! initial values
     ndesi_kri = 0
     nsamp_kri = 0
     nkrig_kri = 0
     nembe_kri = 0
     noffs_kri = 1                      ! matrix offset due to the derivative continuity
     rcova_kri = 0.0_rp
     kfl_covar_kri = 1                  ! power covariance (default)
     nobje_kri = 1

    read(lun_indat_kri,'(a)') words(1) 
     wolab= 'end_' // adjustl(trim(words(1)(1:4))) 
     iread = 1
     do while (iread == 1)
        read(lun_indat_kri,'(a)') woaux
        worea= adjustl(trim(woaux))
        call decodi(woaux,words)        ! decodes the line
        if (adjustl(trim(words(1)(1:5))) == 'desig') then
           read(words(2),'(i4)') ndesi_kri
        else if (adjustl(trim(words(1)(1:5))) == 'objec') then
           read(words(2),'(i4)') mobje_kri
        else if (adjustl(trim(words(1)(1:5))) == 'targe') then
           read(words(2),'(i4)') mobta_kri
        else if (adjustl(trim(words(1)(1:5))) == 'sampl') then
           read(words(2),'(i4)') nsamp_kri
        else if (adjustl(trim(words(1)(1:5))) == 'solve') then
           read(words(2),'(i4)') nsolv_kri
        else if (adjustl(trim(words(1)(1:5))) == 'gridu') then
           read(words(2),'(i4)') mgrid_kri
        else if (adjustl(trim(words(1)(1:5))) == 'nugge') then
           read(words(2),*) nuget_kri
        else if (adjustl(trim(words(1)(1:5))) == 'drift') then
           read(words(2),'(i4)') drift_kri
        else if (adjustl(trim(words(1)(1:5))) == 'covar') then
           if (words(2) == 'power') then
              read(words(3),'(i4)') nauxi
              rcova_kri(1)  = real(nauxi)
              kfl_covar_kri = 1 
           end if
        else if (adjustl(trim(words(1)(1:8))) == adjustl(trim(wolab(1:8))) ) then
           iread = 0
        end if
     end do
     nembe_kri = ndesi_kri + 1      ! dimension of the embedding spa

write (*,*) "111111111111111111111111111111111111111111111111111111"

write (*,*) "ndesi_kri = ", ndesi_kri
write (*,*) "nsamp_kri = ", nsamp_kri
write (*,*) "nkrig_kri = ", nkrig_kri
write (*,*) "nembe_kri = ", nembe_kri
write (*,*) "noffs_kri = ", noffs_kri
write (*,*) "rcova_kri = ", rcova_kri
write (*,*) "kfl_covar_kri = ", kfl_covar_kri
write (*,*) "nobje_kri = ", nobje_kri
write (*,*) "mobje_kri = ", mobje_kri
write (*,*) "mobta_kri = ", mobta_kri
write (*,*) "nsolv_kri = ", nsolv_kri
write (*,*) "mgrid_kri = ", mgrid_kri
write (*,*) "nuget_kri = ", nuget_kri
write (*,*) "drift_kri = ", drift_kri

 case(2) !Block 'Names' 

     !read dimensions and parameters

     read(lun_indat_kri,'(a)') words(1) 

     do iname=1,mobje_kri+ndesi_kri	

	read(lun_indat_kri,'(a)') words(1)
	paran_kri(iname)=trim(words(1))
	
     end do

     read(lun_indat_kri,'(a)') words(1)  

write (*,*) "22222222222222222222222222222222222222222222222222222222"
do iname = 1, mobje_kri+ndesi_kri
  write (*,*) "paran_kri(",i,")=",paran_kri(i)
enddo


  case(3) !Block 'kriging'
     
	     ! read kriged values definition (min,max,ticks des donnees)
	     
	     iread = 1
	     idesi = 1
	     read(lun_indat_kri,'(a)') words(1)
	     nkrig_kri = 1
	     do while (iread == 1)
	        read(lun_indat_kri,*) dkrig_kri(idesi,1),dkrig_kri(idesi,2), nkrdi_kri(idesi) 
	        nkrig_kri = nkrig_kri * nkrdi_kri(idesi) 
	        idesi= idesi+1
	        if (idesi==ndesi_kri+1) then
	           iread= 0
	           read(lun_indat_kri,*) xnbva_kri
	           read(lun_indat_kri,'(a)') words(1)
	        end if
	     end do
	 

	    if (mgrid_kri .EQ. 0) then !If a single point is used instead of a grid
		nkrig_kri=1
	    end if 

write (*,*) "33333333333333333333333333333333333333333333333333333333333"
write (*,*) "nkrig_kri = ", nkrig_kri
write (*,*) "mgrid_kri = ", mgrid_kri
write (*,*) "dkrig_kri(1,1) =", dkrig_kri(1,1)
write (*,*) "dkrig_kri(1,2) =", dkrig_kri(1,2)
write (*,*) "nkrdi_kri(1) = ", nkrdi_kri(1)
write (*,*) "xnbva_kri(1,1) = ", xnbva_kri(1,1)

 case(4)  !Block 'plot' !PIERRE
     
     !read plot values definition varli_kri
     
     iread = 1    
     idesi = 1
     read(lun_indat_kri,'(a)') words(1)

     do while (iread == 1)
        read(lun_indat_kri,*) (varli_kri(idesi,iplot),iplot=1,larpl_kri)
        idesi= idesi+1
        if (idesi==ndesi_kri+1) then
           iread= 0
           read(lun_indat_kri,'(a)') words(1)
        end if
     end do


write (*,*) "4444444444444444444444444444444444444444444444444444444444"
do iread = 1, larpl_kri
write (*,*) "varli_kri(1,",iread,")=",varli_kri(1,iread)
enddo


 case(5) !Block 'samples'
     	
     ! read sampling data
     
write (*,*) "55555555555555555555555555555555555555555555555555555555555"
     iread = 1
     isamp = 1
     read(lun_indat_kri,'(a)') words(1)
     do while (iread == 1)
        read(lun_indat_kri,*) &
             (xsamp_kri(idesi,isamp),idesi=1,ndesi_kri),&
             (xauxi(iobje),iobje=1,mobje_kri)
        
        xobje_kri(1,isamp) = xauxi(mobta_kri)
        
        isamp= isamp+1
        if (isamp==nsamp_kri+1) then
           iread= 0
           read(lun_indat_kri,'(a)') words(1)
        end if
     end do

     do i = 1, isamp-1
        write (*,*) "xobje_kri ", i, "=", xobje_kri(1,i)
        write (*,*) "xsamp_kri ", i, "=", xsamp_kri(1,i)
     enddo
     

  end select

end subroutine kri_indata

