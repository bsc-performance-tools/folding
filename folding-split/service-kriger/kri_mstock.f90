subroutine kri_mstock(itask)

  !-----------------------------------------------------------------------
  !****f* kri_mstock
  ! NAME
  !   kri_mstock
  ! DESCRIPTION
  !-----------------------------------------------------------------------

  use def_kriger
  implicit none
  integer(ip)      :: i,j,k
  integer(ip)      :: itask,naugm
  real(rp)	   :: sumel

if (itask .EQ. 1) then

	! Solver LU:Transform the nxn (naugm=nsamp_kri+1) 'a' matrice (xmaca_kri) into an equivalent LU matrice
	!(in compact form, 'a' is progressively redifined as LU)

	!Number of row/column of the matrice to transormed
	naugm=lntot_kri

	!Initiate the order vector (permutation)

	do i=1,naugm
		order_kri(i,1)=i
	end do

	!Find the highest pivot for the first column
	 call kri_findpi(1)

	!Firts row
	do i=2,naugm
		xmaca_kri(1,i)=xmaca_kri(1,i)/xmaca_kri(1,1)
	end do 

	!The remaining of the matrice
	do i=2,naugm-1
		
		!Element (i,i):
		sumel=0
			do k=1,i-1
				sumel = sumel + (xmaca_kri(i,k)*xmaca_kri(k,i))
			end do 
		xmaca_kri(i,i)=xmaca_kri(i,i)-sumel

		!Colomn 'i' of L 
		do j=i+1,naugm

			sumel=0
			do k=1,i-1
				sumel = sumel + (xmaca_kri(j,k)*xmaca_kri(k,i))
			end do 

			xmaca_kri(j,i)=xmaca_kri(j,i)-sumel
			
		end do

		!Find the highest pivot for the 'i'nd column
		call kri_findpi(i)

		!line 'i' of U
		do j=i+1,naugm

			sumel=0
			do k=1,i-1
				sumel = sumel + (xmaca_kri(i,k)*xmaca_kri(k,j))
			end do 

			xmaca_kri(i,j)=(xmaca_kri(i,j)-sumel)/xmaca_kri(i,i)

		end do 

	end do 

	!Last term
	sumel=0
	do k=1,naugm-1
		sumel = sumel + (xmaca_kri(naugm,k)*xmaca_kri(k,naugm))
	end do 

	xmaca_kri(naugm,naugm)=xmaca_kri(naugm,naugm)-sumel

end if

end subroutine kri_mstock 
