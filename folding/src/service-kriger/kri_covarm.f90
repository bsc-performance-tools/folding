subroutine kri_covarm

  !-----------------------------------------------------------------------
  !****f* kri_covarm
  ! NAME
  !    kri_covarm
  ! DESCRIPTION
  !-----------------------------------------------------------------------

  use def_kriger
  implicit none
  integer(ip)      :: i,j,k,inc
  real(rp)	   :: distc

!Creation of Covariance matrice [xmaco_kri]

do i=1,nsamp_kri
	do j=1,nsamp_kri
		
		!Calcul of distance between two sample
		distc=0
		do k=1,ndesi_kri
			
			distc = distc + (xsamp_kri(k,i)-xsamp_kri(k,j))**2

		end do 

			!calcul of covaraince
			if (kfl_covar_kri .EQ. 1)then !Power covar

				xmaco_kri(i,j)=(sqrt(distc))**rcova_kri(1)  !Co = h^rcova_kri 

				!Nugget effect (smoothing)
				if (i==j) then
					xmaco_kri(i,j)=xmaco_kri(i,j)+nuget_kri
				end if 

				!spheric
				!if (distc < 10000.0) then
				!	xmaco_kri(i,j)=1.5*((sqrt(distc))/10000.0) - 0.5*((sqrt(distc))/10000.0)**3.0
				!else
				!	xmaco_kri(i,j)=0.0
				!end if

				!xmaco_kri(i,j)=(1.0 - exp((-3.0*sqrt(distc)**2.0)/rcova_kri(1)**2.0))


			end if
	end do 
end do


!Augmented Covariance matrice [xmaca_kri]

!Cte drift:
if (drift_kri == 0) then 

	!the ones
	do i=1,nsamp_kri
		xmaca_kri(i,nsamp_kri+1)=1.0
		xmaca_kri(nsamp_kri+1,i)=1.0
	end do

	!the rest
	do i=1,nsamp_kri
		do j=1,nsamp_kri
			xmaca_kri(i,j)=xmaco_kri(i,j)
		end do
	end do

xmaca_kri(nsamp_kri+1,nsamp_kri+1)=0.0
end if 

!linear drift
if (drift_kri == 1) then 

	!the ones
	do i=1,nsamp_kri
		xmaca_kri(i,nsamp_kri+1)=1.0
		xmaca_kri(nsamp_kri+1,i)=1.0
	end do

	!the points 
	do i=1,nsamp_kri
		do j=1,ndesi_kri
			xmaca_kri(i,nsamp_kri+1+j)=xsamp_kri(j,i)
			xmaca_kri(nsamp_kri+1+j,i)=xsamp_kri(j,i)
		end do
	end do

	!the rest
	do i=1,nsamp_kri
		do j=1,nsamp_kri
			xmaca_kri(i,j)=xmaco_kri(i,j)
		end do
	end do

xmaca_kri(nsamp_kri+1,nsamp_kri+1)=0.0
end if 


!quadratic drift
if (drift_kri == 2) then 

	!the ones
	do i=1,nsamp_kri
		xmaca_kri(i,nsamp_kri+1)=1.0
		xmaca_kri(nsamp_kri+1,i)=1.0
	end do

	!the points 
	do i=1,nsamp_kri
		do j=1,ndesi_kri
			xmaca_kri(i,nsamp_kri+1+j)=xsamp_kri(j,i)
			xmaca_kri(nsamp_kri+1+j,i)=xsamp_kri(j,i)
		end do
	end do

	!the points**2 
	do i=1,nsamp_kri
		do j=1,ndesi_kri
			xmaca_kri(i,nsamp_kri+1+ndesi_kri+j)=xsamp_kri(j,i)**2.0
			xmaca_kri(nsamp_kri+1+ndesi_kri+j,i)=xsamp_kri(j,i)**2.0
		end do
	end do

	!The cross products 

	do i=1,nsamp_kri
	inc=1
		do j=1,ndesi_kri-1
			do k=j+1,ndesi_kri
				xmaca_kri(i,nsamp_kri+1+(ndesi_kri*2)+inc)=xsamp_kri(j,i)*xsamp_kri(k,i)
				xmaca_kri(nsamp_kri+1+(ndesi_kri*2)+inc,i)=xsamp_kri(j,i)*xsamp_kri(k,i)
				inc=inc+1
			end do
		end do
	end do	


	!the rest
	do i=1,nsamp_kri
		do j=1,nsamp_kri
			xmaca_kri(i,j)=xmaco_kri(i,j)
		end do
	end do

xmaca_kri(nsamp_kri+1,nsamp_kri+1)=0.0
end if 



end subroutine kri_covarm 
