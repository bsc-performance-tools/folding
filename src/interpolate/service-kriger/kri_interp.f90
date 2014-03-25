subroutine kri_interp

  !-----------------------------------------------------------------------
  !****f* kri_interp
  ! NAME
  !    kri_interp
  ! DESCRIPTION
  !    interpolates using kriging method
  !
  !	- Call the grid fefinition
  !	- Create the gen. covariance matrice: Co = h^3 and the augmented Co matrice ('xmaca_kri')
  !	- Transforme xmaca_kri in a LU matrice. The transormation use the highest pivot of the column
  !	  and an order vector to track the permutations
  !	- For the interpolation, a rhs vector is found for each point of the grid
  !	  and the system is solved by calling 'kri_solsys'
  !	- Finally, the kriging equation is used to find the interpolated point
  !***
  !-----------------------------------------------------------------------
  use def_kriger
  implicit none
  integer(ip)      :: i,j,k,w,naugm,isamp,idesi,inc
  real(rp)	   :: distr,sumkr,sumli,normd,normi


  real(rp),pointer 		::rhsii(:,:),rhsio(:,:)
  real(rp),pointer		::xlamb(:,:)



 allocate(rhsii(lntot_kri,1))   
 rhsii=0.0   
 allocate(rhsio(lntot_kri,1))   
 rhsio=0.0   
 allocate(xlamb(lntot_kri,1))   
 xlamb=0.0   



  ! define the kriging grid
  call kri_defgri

  !Creation of Covariance matrice [xmaco_kri] and Augmented Covariance matrice [xmaca_kri]
  !(one line and one column of [1] are added to xmaco_kri)
  call kri_covarm


  !Transform the augmented covariance matrice [xmaca_kri] in his 'stokage' form, depending
  !on the solver used. 
  !For solver=LU, xmaca_kri is transformed into a an equivalent LU matrice
  !(in compact form, 'a' is progressively redifined as LU)

  call kri_mstock(nsolv_kri)



! INTERPOLATION 

rhsii(:,:)=0.0
xlamb(:,:)=0.0

!Number of row/column of the matrice to transormed
naugm = lntot_kri

 mingl_kri=10e10
 maxgl_kri=-10e10



!A system for each point of the grid
do i=1,nkrig_kri

	!right hand side Covariance 	
	do j=1,nsamp_kri

		distr=0
		do k=1,ndesi_kri
			distr = distr + (xsamp_kri(k,j)-xkrig_kri(k,i))**2
		end do 
		
		!calcul of covariance
		if (kfl_covar_kri .EQ. 1)then !Power covar
		

			rhsii(j,1)=(sqrt(distr))**rcova_kri(1)  !Co = h^rcova_kri 

			!spheric
			!if (distr < 10000.0) then
			!	rhsii(j,1)=1.5*((sqrt(distr))/10000.0) - 0.5*((sqrt(distr))/10000.0)**3.0
			!else
			!	rhsii(j,1)=0.0
			!end if

			!rhsii(j,1)=(1.0 - exp((-3.0*sqrt(distr)**2.0)/rcova_kri(1)**2.0))

		end if

		
	end do

	!Rest of the rhi vector
	rhsii(nsamp_kri+1,1)=1

	if (drift_kri==1) then
		do k=1,ndesi_kri
			rhsii(nsamp_kri+1+k,1)=xkrig_kri(k,i)
		end do

	end if

	if (drift_kri==2) then
		do k=1,ndesi_kri
			rhsii(nsamp_kri+1+k,1)=xkrig_kri(k,i)
		end do

		do k=1,ndesi_kri
			rhsii(nsamp_kri+1+ndesi_kri+k,1)=xkrig_kri(k,i)**2.0
		end do

		inc=1
		do k=1,ndesi_kri-1
			do w=k+1,ndesi_kri
				rhsii(nsamp_kri+1+(ndesi_kri*2)+inc,1)=xkrig_kri(k,i)*xkrig_kri(w,i)
				inc=inc+1
			end do
		end do


	end if


	!Reorder rhsii about order vector
	do k=1,naugm
		rhsio(k,1)=rhsii(int(order_kri(k,1)),1)
	end do 


	!Solve the system with respect to xlamb
	call kri_solsys(rhsio,xlamb)


	!write(*,*) 'xlamb ',xlamb


	!Kriging equation
	sumkr=0.0
	do isamp=1,nsamp_kri
		sumkr=sumkr + xlamb(isamp,1)*xobje_kri(1,isamp)
	end do
		
	!Fill the solution matrice 'kriso_kri'
	do idesi=1,ndesi_kri
		kriso_kri(idesi,i)=xkrig_kri(idesi,i)
	end do

	kriso_kri(ndesi_kri+1,i)=sumkr

	!Track min and max values
	if (sumkr<mingl_kri) then
		mingl_kri=sumkr
		
		do idesi=1,ndesi_kri
			minpt_kri(idesi)=xkrig_kri(idesi,i)
		end do

	end if

	if (sumkr>maxgl_kri) then
		maxgl_kri=sumkr

		do idesi=1,ndesi_kri
			maxpt_kri(idesi)=xkrig_kri(idesi,i)
		end do


	end if



end do 

  


deallocate(rhsii)
deallocate(rhsio)
deallocate(xlamb)

end subroutine kri_interp
