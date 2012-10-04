program kriger
  !-----------------------------------------------------------------------
  !****f* kriger
  ! NAME
  !    kriger
  ! DESCRIPTION
  !    kriging interpolation program
  !***
  !-----------------------------------------------------------------------
  use      def_kriger    ! global variables definition

  write(6,*) '--|  '
  write(6,*) '--| Kriger: Multidimensional interpolation tool'
  write(6,*) '--|  '
  write(6,*) '--|    Opening files'
  call kri_openfi(1)     ! opening files 
  write(6,*) '--|    Reading data'

  call kri_indata(1)     ! reading data from input file 
  call kri_memall(1)     ! allocate memory

  call kri_indata(2)     
  call kri_indata(3)     
  call kri_memall(3)
  call kri_indata(4)     
  call kri_indata(5) 

  write(6,*) '--|    Computing...'
  call kri_interp        ! interpolate
  call kri_oudata        ! output results
  call kri_openfi(2)     ! closing files
  call kri_memall(10)    ! deallocate memory
  write(6,*) '--|    Results dumped on kriger.out file'
  write(6,*) '--|    Batch gnuplot file is kriger.plo'
  write(6,*) '--|   '
  write(6,*) '--| Bye'
  write(6,*) '--|   '

end program kriger
