#include <gbpCommon.h>
#include <gbpSID.h>

void calc_sum_global(void   *data,
                     void   *result,
       	             size_t  n_data,
                     SID_Datatype  type,
                     int           mode,
                     SID_Comm     *comm){
  int          i_data;
  double       d_temp2;
  double       d_temp;
  float        f_temp;
  int          i_temp;
  unsigned int ui_temp;
  size_t       s_temp;
  long long    l_temp;

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
        ui_temp+=IABS(((int *)data)[i_data]);
    else
      for(i_data=0;i_data<n_data;i_data++)
        ui_temp+=((int *)data)[i_data];
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
        l_temp+=((long long *)data)[i_data];
    else
      for(i_data=0;i_data<n_data;i_data++)
        l_temp+=((long long *)data)[i_data];
  }
  else
    SID_trap_error("Unknown variable type in calc_sum",ERROR_LOGIC);

  if(check_mode_for_flag(mode,CALC_MODE_RETURN_DOUBLE)){
    if(type==SID_DOUBLE)
      d_temp2=(double)d_temp;
    else if(type==SID_FLOAT)
      d_temp2=(double)f_temp;
    else if(type==SID_INT)
      d_temp2=(double)i_temp;
    else if(type==SID_UNSIGNED)
      d_temp2=(double)ui_temp;
    else if(type==SID_SIZE_T)
      d_temp2=(double)s_temp;
    else if(type==SID_LONG_LONG)
      d_temp2=(double)l_temp;
    else
      SID_trap_error("Unknown variable type in calc_sum",ERROR_LOGIC);
    SID_Allreduce(&d_temp2,result,1,SID_DOUBLE,SID_SUM,comm);
  }
  else{
    if(type==SID_DOUBLE)
      SID_Allreduce(&d_temp,result,1,SID_DOUBLE,SID_SUM,comm);
    else if(type==SID_FLOAT)
      SID_Allreduce(&f_temp,result,1,SID_FLOAT, SID_SUM,comm);
    else if(type==SID_INT)
      SID_Allreduce(&i_temp,result,1,SID_INT,   SID_SUM,comm);
    else if(type==SID_UNSIGNED)
      SID_Allreduce(&ui_temp,result,1,SID_UNSIGNED,SID_SUM,comm);
    else if(type==SID_SIZE_T)
      SID_Allreduce(&s_temp,result,1,SID_SIZE_T,SID_SUM,comm);
    else if(type==SID_LONG_LONG)
      SID_Allreduce(&l_temp,result,1,SID_LONG_LONG,SID_SUM,comm);
    else
      SID_trap_error("Unknown variable type in calc_sum",ERROR_LOGIC);
  }
}

