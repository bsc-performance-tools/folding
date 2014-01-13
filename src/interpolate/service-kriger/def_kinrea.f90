module def_kinrea

  !-----------------------------------------------------------------------
  !****f* def_kinrea
  ! NAME
  !    def_kinrea
  ! DESCRIPTION
  !    integer and real types definition
  !***
  !-----------------------------------------------------------------------

  integer, parameter  :: ip = selected_int_kind(9)   ! 4-byte integer
  integer, parameter  :: rp = kind(1.0d0)            ! double precision
  

end module def_kinrea
