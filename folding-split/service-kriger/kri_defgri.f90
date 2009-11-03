subroutine kri_defgri
  !-----------------------------------------------------------------------
  !****f* kri_defgri
  ! NAME
  !    kri_defgri
  ! DESCRIPTION
  !    define the kriging grid
  !***
  !  Check also this alternative (mauricios)
  !  II = mod( id_block, NBZ-2 ) + 2
  !  JJ = mod( ( id_block / (NBZ-2) ), (NBX-2) ) + 2
  !  KK = mod( ( id_block / ((NBZ-2) * (NBX-2)) ), (NBY-2) ) + 2
  !-----------------------------------------------------------------------
  use def_kriger
  implicit none
  integer(ip)      :: &
       idesi,ikrig,ikpro(50),ikrdi(3,50),inogr
  real(rp)         :: &
       xides,xjdes,rinva

if (mgrid_kri .EQ. 1) then
	  ! define products and total number of kriged points
	  ikpro    = 1
	  ikrdi    = 0
	  ikpro(1) = nkrdi_kri(1)
	  do idesi= 2,ndesi_kri
	     ikpro(idesi)= ikpro(idesi-1) * nkrdi_kri(idesi)
	  end do

	  do ikrig = 1,nkrig_kri     
	     ikrdi(1,1) = mod(ikrig-1,ikpro(1))
	     rinva= (dkrig_kri(1,2)-dkrig_kri(1,1)) / real(nkrdi_kri(1)-1)        
	     xkrig_kri(1,ikrig) = dkrig_kri(1,1)+ real(ikrdi(1,1)) * rinva 

	     do idesi= 2,ndesi_kri
	        ikrdi(1,idesi) = mod((ikrig-1),ikpro(idesi)) / ikpro(idesi-1)        
	        rinva= (dkrig_kri(idesi,2)-dkrig_kri(idesi,1)) / real(nkrdi_kri(idesi)-1)        
	        xkrig_kri(idesi,ikrig) = dkrig_kri(idesi,1) + real(ikrdi(1,idesi)) * rinva
	     end do

	     !write(6,*) ikrig,(xkrig_kri(idesi,ikrig),idesi=1,ndesi_kri)    

	  end do

end if


if (mgrid_kri .EQ. 0) then
	do inogr=1,ndesi_kri	
		xkrig_kri(inogr,1)=xnbva_kri(1,inogr)
	end do
end if 

end subroutine kri_defgri

