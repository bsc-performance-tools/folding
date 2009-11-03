subroutine call_Kriger (num_in_samples, X_samples, Y_samples, num_out_samples, Y_out_samples, min_value, max_value)

  use def_kriger
  implicit none
  
  integer :: i
  integer(ip), intent(in)   :: num_in_samples
  integer(ip), intent(in)   :: num_out_samples
  real(rp), intent(in)      :: X_samples(1:num_in_samples)
  real(rp), intent(in)      :: Y_samples(1:num_in_samples)
  real(rp), intent(out)     :: Y_out_samples(1:num_out_samples)
  real(rp), intent(in)     :: min_value
  real(rp), intent(in)     :: max_value

! real(rp), pointer

!  call kri_indata(1)     ! reading data from input file 

 ndesi_kri =            1
 nsamp_kri =         num_in_samples
 nkrig_kri =            0
 nembe_kri =            2
 noffs_kri =            1
 rcova_kri(1) =    3.00000000000000     
 kfl_covar_kri =            1
 nobje_kri =            1
 mobje_kri =            1
 mobta_kri =            1
 nsolv_kri =            1
 mgrid_kri =            1
! nuget_kri =   1.000000000000000E-005
 nuget_kri =    1.000000000000000E-004
 drift_kri =            1

     call kri_memall(1)     ! allocate memory


     ! call kri_indata(2)     
!     paran_kri(1) = "heel"
!     paran_kri(2) = "Cl"

     ! call kri_indata(3)

 nkrig_kri =         num_out_samples
 mgrid_kri =            1
 dkrig_kri(1,1) =   min_value ! 0.00000000000000     
 dkrig_kri(1,2) =   max_value ! 1.00000000000000     
 nkrdi_kri(1) =         num_out_samples

     call kri_memall(3)

     ! call kri_indata(4)
     varli_kri(1,1)=0.0
   
     ! call kri_indata(5)
     do i = 1, num_in_samples
       xobje_kri(1,i) = Y_samples(i)
       xsamp_kri(1,i) = X_samples(i)
     enddo

!     do i = 1, num_in_samples
!        write (*,*) "xobje_kri ", i, "=", xobje_kri(1,i)
!        write (*,*) "xsamp_kri ", i, "=", xsamp_kri(1,i)
!     enddo

     call kri_interp        ! interpolate

     ! call kri_oudata        ! output results
     do i=1,num_out_samples
!        write (*,*) "kriso_kri(2,",i,")=",kriso_kri(2,i)
        Y_out_samples(i) = kriso_kri(2,i)
     end do

     call kri_memall(10) ! free memory

end subroutine call_Kriger
