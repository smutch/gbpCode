#include <gbpCommon.h>
#include <gbpSID.h>

void calc_sum(void   *data,
              void   *result,
    	      size_t  n_data,
              SID_Datatype  type,
              int           mode){
  int          i_data;
  double       d_temp;
  float        f_temp;
  int          i_temp;
  unsigned int ui_temp;
  size_t       s_temp;
  long long    l_temp;
  if(n_data<1){
    if(type==SID_DOUBLE || check_mode_for_flag(mode,CALC_MODE_RETURN_DOUBLE))
      ((double *)result)[0]=0.;
    else if(type==SID_FLOAT)
      ((float  *)result)[0]=0.;
    else if(type==SID_INT)
      ((int    *)result)[0]=0;
    else if(type==SID_UNSIGNED)
      ((unsigned int *)result)[0]=0;
    else if(type==SID_SIZE_T)
      ((size_t *)result)[0]=0;
    else if(type==SID_LONG_LONG)
      ((long long *)result)[0]=0;
    else
      SID_trap_error("Unknown variable type in calc_sum",ERROR_LOGIC);
  }
  if(type==SID_DOUBLE){
    d_temp=0.;
    if(check_mode_for_flag(mode,CALC_MODE_ABS))
      for(i_data=0;i_data<n_data;i_data++)
        d_temp+=IABS(((double *)data)[i_data]);
    else
      for(i_data=0;i_data<n_data;i_data++)
        d_temp+=((double *)data)[i_data];
  }
  else if(type==SID_FLOAT){
    f_temp=0.;
    if(check_mode_for_flag(mode,CALC_MODE_ABS))
      for(i_data=0;i_data<n_data;i_data++)
        f_temp+=IABS(((float *)data)[i_data]);
    else
      for(i_data=0;i_data<n_data;i_data++)
        f_temp+=((float *)data)[i_data];
  }
  else if(type==SID_INT){
    i_temp=0;
    if(check_mode_for_flag(mode,CALC_MODE_ABS))
      for(i_data=0;i_data<n_data;i_data++)
        i_temp+=IABS(((int *)data)[i_data]);
    else
      for(i_data=0;i_data<n_data;i_data++)
        i_temp+=((int *)data)[i_data];
  }
  else if(type==SID_UNSIGNED){
    ui_temp=0;
    if(check_mode_for_flag(mode,CALC_MODE_ABS))
      for(i_data=0;i_data<n_data;i_data++)
        ui_temp+=((unsigned int *)data)[i_data];
    else
      for(i_data=0;i_data<n_data;i_data++)
        ui_temp+=((unsigned int *)data)[i_data];
  }
  else if(type==SID_SIZE_T){
    s_temp=0;
    if(check_mode_for_flag(mode,CALC_MODE_ABS))
      for(i_data=0;i_data<n_data;i_data++)
        s_temp+=((size_t *)data)[i_data];
    else
      for(i_data=0;i_data<n_data;i_data++)
        s_temp+=((size_t *)data)[i_data];
  }
  else if(type==SID_LONG_LONG){
    l_temp=0;
    if(check_mode_for_flag(mode,CALC_MODE_ABS))
      for(i_data=0;i_data<n_data;i_data++)
        s_temp+=((long long *)data)[i_data];
    else
      for(i_data=0;i_data<n_data;i_data++)
        l_temp+=((long long *)data)[i_data];
  }
  else
    SID_trap_error("Unknown variable type in calc_sum",ERROR_LOGIC);

  if(type==SID_DOUBLE || check_mode_for_flag(mode,CALC_MODE_RETURN_DOUBLE))
    ((double *)result)[0]=d_temp;
  else if(type==SID_FLOAT)
    ((float  *)result)[0]=f_temp;
  else if(type==SID_INT)
    ((int    *)result)[0]=i_temp;
  else if(type==SID_UNSIGNED)
    ((unsigned int *)result)[0]=ui_temp;
  else if(type==SID_SIZE_T)
    ((size_t *)result)[0]=s_temp;
  else if(type==SID_LONG_LONG)
    ((long long *)result)[0]=l_temp;
  else
    SID_trap_error("Unknown variable type in calc_sum",ERROR_LOGIC);
}

