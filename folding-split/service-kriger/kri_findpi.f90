subroutine kri_findpi(jcolu)

  !-----------------------------------------------------------------------
  !****f* kri_findpivot
  ! NAME
  !    kri_findpivot
  ! DESCRIPTION
  !	Called by 'kri_interp.f90'
  !	Find the highest value in the 'jcolu' column of a matrice and rearanged the
  !	matrice 'xmaca_kri' (actualy the LU matrice) and the order vertor 'order_kri'
  !
  !	pospi : Position of the pivot
  !	ordbu : Order bakup 
  !***
  !-----------------------------------------------------------------------

  use def_kriger
  implicit none
  
  integer(ip)      		:: i,numrc,pospi,ordbu
  integer(ip),intent(in)        :: jcolu
  real(rp)	   		:: biger,nexti,bakup
 


 !Number of row/column of the matrice to transormed
 numrc=lntot_kri


!Find the higher number below the diagonal 
biger=0.0
pospi=jcolu

do i=jcolu,numrc

	nexti=abs(xmaca_kri(i,jcolu))

	if (nexti .GT. biger) then
		biger=nexti
		pospi=i
	end if
end do

!Switch the lines of the LU matrice

do i=1,numrc

	bakup=xmaca_kri(pospi,i)
	xmaca_kri(pospi,i)=xmaca_kri(jcolu,i)
	xmaca_kri(jcolu,i)=bakup

end do 

!Modify the order vector
ordbu=order_kri(pospi,1)
order_kri(pospi,1)=order_kri(jcolu,1)
order_kri(jcolu,1)=ordbu

end subroutine kri_findpi
