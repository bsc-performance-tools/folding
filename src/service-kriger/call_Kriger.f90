subroutine call_kriger_region (num_in_samples, X_samples, Y_samples, num_out_samples, Y_out_samples, min_value, max_value, &
    inp_nuget_kri)
  use def_kriger
  implicit none
  integer :: i
  integer(ip), intent(in) :: num_in_samples
  integer(ip), intent(in) :: num_out_samples
  real(rp), intent(in) :: X_samples(1:num_in_samples)
  real(rp), intent(in) :: Y_samples(1:num_in_samples)
  real(rp), intent(out) :: Y_out_samples(1:num_out_samples)
  real(rp), intent(in) :: min_value
  real(rp), intent(in) :: max_value
  real(rp), intent(in) :: inp_nuget_kri

  ndesi_kri = 1
  nsamp_kri = num_in_samples
  nkrig_kri = 0
  nembe_kri = 2
  noffs_kri = 1
  rcova_kri(1) = 3.00000000000000     
  kfl_covar_kri = 1
  nobje_kri = 1
  mobje_kri = 1
  mobta_kri = 1
  nsolv_kri = 1
  mgrid_kri = 1
  nuget_kri = inp_nuget_kri
  drift_kri = 1

  call kri_memall(1) ! allocate memory

  nkrig_kri = num_out_samples
  mgrid_kri = 1
  dkrig_kri(1,1) = min_value
  dkrig_kri(1,2) = max_value
  nkrdi_kri(1) = num_out_samples

  call kri_memall(3)

  !varli_kri(1,1)=0.0
  varli_kri=0.0
  do i = 1, num_in_samples
    xobje_kri(1,i) = Y_samples(i)
    xsamp_kri(1,i) = X_samples(i)
  enddo

  call kri_interp ! interpolate

  do i=1,num_out_samples
    Y_out_samples(i) = kriso_kri(2,i)
  end do

  call kri_memall(10) ! free memory

end 

subroutine call_kriger_point (num_in_samples, X_samples, Y_samples, X_value, Y_out_value, min_value, max_value, &
    inp_nuget_kri)
  use def_kriger
  implicit none
  integer :: i
  integer(ip), intent(in) :: num_in_samples
  real(rp), intent(in) :: X_samples(1:num_in_samples)
  real(rp), intent(in) :: Y_samples(1:num_in_samples)
  real(rp), intent(in) :: X_value
  real(rp), intent(out) :: Y_out_value
  real(rp), intent(in) :: min_value
  real(rp), intent(in) :: max_value
  real(rp), intent(in) :: inp_nuget_kri

  ndesi_kri = 1
  nsamp_kri = num_in_samples
  nkrig_kri = 1
  nembe_kri = 2
  noffs_kri = 1
  rcova_kri(1) = 3.00000000000000     
  kfl_covar_kri = 1
  nobje_kri = 1
  mobje_kri = 1
  mobta_kri = 1
  nsolv_kri = 1
  mgrid_kri = 0
  nuget_kri = inp_nuget_kri
  drift_kri = 1

  call kri_memall(1) ! allocate memory

  dkrig_kri(1,1) = min_value
  dkrig_kri(1,2) = max_value
  nkrdi_kri(1) = 1
  xnbva_kri(1,1) = X_value

  call kri_memall(3)

  !varli_kri(1,1)=0.0
  varli_kri=0.0
  do i = 1, num_in_samples
    xobje_kri(1,i) = Y_samples(i)
    xsamp_kri(1,i) = X_samples(i)
  enddo

  call kri_interp ! interpolate

  Y_out_value = kriso_kri(2,1)

  call kri_memall(10) ! free memory
end
