subroutine kri_memall(itask)
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
  integer(ip)      :: itask,istat,inext,idesi,i
  real(rp)         :: temp


     
 !The number of lines/columns in the xmaca
 if (drift_kri==0) then 
	lntot_kri=nsamp_kri+1	
 end if

 if (drift_kri==1) then
	lntot_kri=nsamp_kri+1+ndesi_kri	
 end if

 if (drift_kri==2) then

	 temp=0
	 do i=1,ndesi_kri-1
		temp=temp+i
	 end do

	lntot_kri=nsamp_kri+1+(ndesi_kri*2)+temp	
 end if



 select case(itask)

  case(1)

     allocate(xmaca_kri(lntot_kri,lntot_kri),stat=istat) 
     xmaca_kri=0.0_rp
     allocate(order_kri(lntot_kri,1),stat=istat)
     order_kri=0.0_rp

     allocate(xmaco_kri(nsamp_kri,nsamp_kri),stat=istat)   
     xmaco_kri=0.0_rp
     allocate(xsamp_kri(ndesi_kri,nsamp_kri),stat=istat)
     xsamp_kri=0.0_rp
     allocate(minpt_kri(ndesi_kri),stat=istat)
     minpt_kri=0.0_rp
     allocate(maxpt_kri(ndesi_kri),stat=istat)
     maxpt_kri=0.0_rp
     allocate(xobje_kri(nobje_kri,nsamp_kri),stat=istat)
     xobje_kri=0.0_rp
     allocate(dkrig_kri(ndesi_kri,        2),stat=istat)
     dkrig_kri=0.0_rp
     allocate(nkrdi_kri(ndesi_kri          ),stat=istat)
     nkrdi_kri=0
     allocate(xnbva_kri(1,ndesi_kri),stat=istat)
     xnbva_kri=0.0_rp
     allocate(paran_kri(mobje_kri+ndesi_kri),stat=istat)


  case(3)
     allocate(xkrig_kri(ndesi_kri,nkrig_kri),stat=istat)
     xkrig_kri=0.0_rp
     allocate(kriso_kri(ndesi_kri+1,nkrig_kri),stat=istat)	
     kriso_kri=0.0_rp
     allocate(xkrob_kri(nobje_kri,nkrig_kri),stat=istat)
     xkrob_kri=0.0_rp
   

     larpl_kri=0.0	
     do idesi=1,ndesi_kri
	inext=xnbva_kri(1,idesi)
	if (inext .GT. larpl_kri) then 
		larpl_kri=inext
	end if
     end do

     allocate(varli_kri(ndesi_kri,larpl_kri),stat=istat)
     varli_kri=0.0_rp 

 
  case(10)
     deallocate(xsamp_kri,stat=istat)
     deallocate(xkrig_kri,stat=istat)
     deallocate(dkrig_kri,stat=istat)
     deallocate(nkrdi_kri,stat=istat)
     deallocate(paran_kri,stat=istat)

     deallocate (varli_kri, stat=istat)
     deallocate (xkrob_kri, stat=istat)
     deallocate (kriso_kri, stat=istat)
     deallocate (xnbva_kri, stat=istat)
     deallocate (xobje_kri, stat=istat)
     deallocate (maxpt_kri, stat=istat)
     deallocate (minpt_kri, stat=istat)
     deallocate (xmaco_kri, stat=istat)
     deallocate (xmaca_kri, stat=istat)
     deallocate (order_kri, stat=istat)

  end select

end subroutine kri_memall
