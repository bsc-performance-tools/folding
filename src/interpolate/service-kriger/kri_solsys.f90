subroutine kri_solsys(rhsio,xlamb)

  !-----------------------------------------------------------------------
  !****f* kri_solsys2
  ! NAME
  !    kri_solsys2
  ! DESCRIPTION
  !	Called by 'kri_interp.f90'
  !	solve the system Ax = b (for each grid location)
  !	where: 
  !		A : 'xmaca_kri', the covariance matrice, transformed as a LU matrice. 
  !		    (this matrice is the same for all the grid location)
  !		x : 'xlamb', the unkowns matrice. The xlamb's are the coefficient of the kriging equation
  !		     The vector is return to 'kri_interp.f90'
  !		b : 'rhsio', the rhs covariance term for this grid location.
  !		     The vector is given by 'kri_interp.f90'
  !
  !	The sytem is then solve by forward and backward substitution. 
  !
  !	rhsio : Right hand side ordonate 
  !***
  !-----------------------------------------------------------------------

  use def_kriger
  implicit none
  
  integer(ip)      :: i,j,k,numrc
  real(rp)	   :: sumel

  real(rp),dimension(lntot_kri,1) 				::ytemp
  real(rp),dimension(lntot_kri,1),intent(in) 			::rhsio
  real(rp),dimension(lntot_kri,1),intent(out) 		::xlamb

  xlamb(:,:)=0.0
  ytemp(:,:)=0.0

  numrc=lntot_kri !Number of row/column of the matrice to invert

!forward substitution to solve L*y=rhsio

ytemp(1,1)=rhsio(1,1)/xmaca_kri(1,1)

do i=2,numrc

	sumel=0
    !$OMP PARALLEL DO REDUCTION(-:sumel) SHARED(xmaca_kri,ytemp,i)
	do k=1,i-1
		sumel = sumel - (xmaca_kri(i,k)*ytemp(k,1))
	end do
	
	ytemp(i,1)=(rhsio(i,1)+sumel)/xmaca_kri(i,i)

end do

!backward substitution to solve Ux=y

xlamb(numrc,1)=ytemp(numrc,1)

do i=1,numrc-1
	sumel=0
    !$OMP PARALLEL DO REDUCTION(-:sumel) SHARED(xmaca_kri,xlamb,i,numrc)
	do k=1,i
		sumel = sumel -xmaca_kri(numrc-i,(numrc-i)+k)*xlamb((numrc-i)+k,1)
	end do

	xlamb(numrc-i,1)=ytemp(numrc-i,1)+sumel

end do 


end subroutine kri_solsys
