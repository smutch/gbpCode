#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <gbpLib.h>
#include <gbpMath.h>
#include <gbpHalos.h>
#include <gbpTrees.h>

void write_match_results(char       *filename_out_dir,
                         char       *filename_out_root,
                         int         i_read,
                         int         j_read,
                         plist_info *plist1,
                         plist_info *plist2,
                         int         k_match){
  char        filename_out[256];
  FILE       *fp_out;
  int         k_read,l_read;
  int         flag_go;
  int         i_read_start_file;
  int         i_read_stop_file;
  int         i_read_step_file;
  int         n_search_file;
  int         n_search_total;
  int         n_k_match;
  int         flag_match_subgroups;
  char        group_text_prefix[5];
  int         n_matches;
  int         n_files;
  char        filename_cat1[256];
  char        filename_cat2[256];
  int         n_groups_1;
  int         n_groups_1_local;
  int         n_groups_2;
  int         n_groups_2_local;
  int         i_group;
  int         buffered_count;
  int         buffered_count_local;
  int         j_group;
  int         index_test;
  int         i_rank;
  int        *n_particles;
  int        *n_sub_group;
  int        *file_index_1;
  int        *match_id   =NULL;
  float      *match_score=NULL;
  char        cat_name_1[20];
  char        cat_name_2[20];
  size_t     *match_rank =NULL;
  size_t     *match_index=NULL;
  size_t      offset;
  int        *n_return;
  void       *buffer;
  int        *buffer_int;
  size_t     *buffer_size_t;
  float      *buffer_float;
  int         n_buffer_max=4096; // 32k for 8-byte values
  int         n_buffer;
  int         i_buffer;
  int         j_buffer;

  // Set catalog names
  sprintf(filename_cat1,"%03d",i_read);
  sprintf(filename_cat2,"%03d",j_read);

  switch(k_match){
     case 0:
        flag_match_subgroups=MATCH_SUBGROUPS;
        sprintf(group_text_prefix,"sub");
        break;
     case 1:
        flag_match_subgroups=MATCH_GROUPS;
        sprintf(group_text_prefix,"");
        break;
  }

  // Set filename and open file
  if(filename_out_dir!=NULL)
     sprintf(filename_out,"%s/%s_%s_%s.%sgroup_matches",filename_out_dir,filename_out_root,filename_cat1,filename_cat2,group_text_prefix);
  else
     sprintf(filename_out,"%s_%s_%s.%sgroup_matches",filename_out_root,filename_cat1,filename_cat2,group_text_prefix);

  SID_log("Writing match results to {%s}...",SID_LOG_OPEN|SID_LOG_TIMER,filename_out);                 

  // Fetch catalog and matching info ...
  n_groups_1      =((int   *)ADaPS_fetch(plist1->data,"n_%sgroups_all_%s",     group_text_prefix,filename_cat1))[0];
  n_groups_1_local=((int   *)ADaPS_fetch(plist1->data,"n_%sgroups_%s",         group_text_prefix,filename_cat1))[0];
  file_index_1    = (int   *)ADaPS_fetch(plist1->data,"file_index_%sgroups_%s",group_text_prefix,filename_cat1);
  n_groups_2      =((int   *)ADaPS_fetch(plist2->data,"n_%sgroups_all_%s",     group_text_prefix,filename_cat2))[0];
  n_groups_2_local=((int   *)ADaPS_fetch(plist2->data,"n_%sgroups_%s",         group_text_prefix,filename_cat2))[0];
  match_id        = (int   *)ADaPS_fetch(plist1->data,"match_match");
  match_score     = (float *)ADaPS_fetch(plist1->data,"match_score_match");

  // Generate ranking of matches
  SID_set_verbosity(SID_SET_VERBOSITY_RELATIVE,0);
  sort(match_id,   (size_t)n_groups_1_local,&match_index,SID_INT,   SORT_GLOBAL,SORT_COMPUTE_INDEX,SORT_COMPUTE_NOT_INPLACE);
  sort(match_index,(size_t)n_groups_1_local,&match_rank, SID_SIZE_T,SORT_GLOBAL,SORT_COMPUTE_INDEX,SORT_COMPUTE_NOT_INPLACE);
  SID_free(SID_FARG match_index);
  SID_set_verbosity(SID_SET_VERBOSITY_DEFAULT);

  // ... write header ...
  if(SID.I_am_Master){
     if(filename_out_dir!=NULL)
        mkdir(filename_out_dir,02755);
     if((fp_out=fopen(filename_out,"w"))==NULL)
        SID_trap_error("Could not open {%s} for writing.",ERROR_IO_OPEN,filename_out);
     fwrite(&i_read,    sizeof(int),1,fp_out);
     fwrite(&j_read,    sizeof(int),1,fp_out);
     fwrite(&n_groups_1,sizeof(int),1,fp_out);
     fwrite(&n_groups_2,sizeof(int),1,fp_out);
  }

  // Now we write the matching results.  We need to write back to the file in the
  //   order that it was read from the halo catalogs, not necessarily the PH order
  //   that it is stored in RAM.  This requires some buffer trickery.
  buffer       =SID_malloc(n_buffer_max*sizeof(size_t));
  buffer_int   =(int    *)buffer;
  buffer_size_t=(size_t *)buffer;
  buffer_float =(float  *)buffer;

  // Write match_ids ...
  //    ... loop over all the groups in buffer-sized batches
  SID_log("Writing match IDs...",SID_LOG_OPEN);                 
/*
FILE *fp_test;
char  filename_test[256];
size_t buffer_test[10000];
sprintf(filename_test,"test_%sgroup_match_IDs_%s_%s.dat",group_text_prefix,filename_cat1,filename_cat2);
if(SID.I_am_Master) fp_test=fopen(filename_test,"w");
*/
  for(i_group=0,buffered_count_local=0;i_group<n_groups_1;i_group+=n_buffer){
     // Decide this buffer iteration's size
     n_buffer=MIN(n_buffer_max,n_groups_1-i_group);
     // Set the buffer to a default value smaller than the smallest possible data size
     for(i_buffer=0;i_buffer<n_buffer;i_buffer++)
        buffer_int[i_buffer]=-2; // Min value of match_id is -1
/*
for(i_buffer=0;i_buffer<n_buffer;i_buffer++)
   buffer_test[i_buffer]=0;
*/
     // Determine if any of the local data is being used for this buffer
     for(j_group=0;j_group<n_groups_1_local;j_group++){
        index_test=file_index_1[j_group]-i_group;
        // ... if so, set the appropriate buffer value
        if(index_test>=0 && index_test<n_buffer){
          buffer_int[index_test]=match_id[j_group];
/*
buffer_test[index_test]=match_rank[j_group];
*/
          buffered_count_local++;
        }
     }
     // Doing a global max on the buffer yields the needed buffer on all ranks
     SID_Allreduce(SID_IN_PLACE,buffer_int,n_buffer,SID_INT,SID_MAX,SID.COMM_WORLD);
/*
SID_Allreduce(SID_IN_PLACE,buffer_test,n_buffer,SID_SIZE_T,SID_MAX,SID.COMM_WORLD);
*/
     // Sanity check
     for(i_buffer=0;i_buffer<n_buffer;i_buffer++){
       if(buffer_int[i_buffer]<-1)
         SID_trap_error("Illegal match_id result (%d) for group No. %d.",ERROR_LOGIC,buffer_int[i_buffer],i_group+i_buffer);
     }
     // Write the buffer
     if(SID.I_am_Master){
       fwrite(buffer_int,sizeof(int),(size_t)n_buffer,fp_out);
/*
for(i_buffer=0;i_buffer<n_buffer;i_buffer++) fprintf(fp_test,"%7d %7d %7lld\n",i_group+i_buffer,buffer_int[i_buffer],buffer_test[i_buffer]);
*/
     }
  }
  SID_log("Done.",SID_LOG_CLOSE);
/*
if(SID.I_am_Master) fclose(fp_test);
*/

  // Sanity check
  calc_sum_global(&buffered_count_local,&buffered_count,1,SID_INT,CALC_MODE_DEFAULT,SID.COMM_WORLD);
  if(buffered_count!=n_groups_1)
    SID_trap_error("Buffer counts don't make sense (ie %d!=%d) after writing match IDs.",ERROR_LOGIC,buffered_count,n_groups_1);

  // Write match sort indices ...
  SID_log("Writing match sort indices...",SID_LOG_OPEN);                 
  //    ... loop over all the groups in buffer-sized batches
/*
sprintf(filename_test,"test_%sgroup_match_index_%s_%s.dat",group_text_prefix,filename_cat1,filename_cat2);
if(SID.I_am_Master) fp_test=fopen(filename_test,"w");
*/
  for(i_group=0,buffered_count_local=0;i_group<n_groups_1;i_group+=n_buffer){
     // Decide this buffer iteration's size
     n_buffer=MIN(n_buffer_max,n_groups_1-i_group);
     // Set the buffer to a default value smaller than the smallest possible data size
     for(i_buffer=0;i_buffer<n_buffer;i_buffer++)
        buffer_size_t[i_buffer]=0; // Min value of match_rank is 0
     // Determine if any of the local data is being used for this buffer
     for(j_group=0;j_group<n_groups_1_local;j_group++){
        index_test=match_rank[j_group]-i_group;
        // ... if so, set the appropriate buffer value
        if(index_test>=0 && index_test<n_buffer){
          buffer_size_t[index_test]=file_index_1[j_group];
          buffered_count_local++;
        }
     }
     // Doing a global max on the buffer yields the needed buffer on all ranks
     SID_Allreduce(SID_IN_PLACE,buffer_size_t,n_buffer,SID_SIZE_T,SID_MAX,SID.COMM_WORLD);
     // Sanity check
     for(i_buffer=0;i_buffer<n_buffer;i_buffer++){
       if(buffer_size_t[i_buffer]<0)
         SID_trap_error("Illegal match_rank result (%lld) for group No. %d.",ERROR_LOGIC,buffer_size_t[i_buffer],i_group+i_buffer);
     }
     // Write the buffer
     if(SID.I_am_Master){
       fwrite(buffer,sizeof(size_t),(size_t)n_buffer,fp_out);
/*
for(i_buffer=0;i_buffer<n_buffer;i_buffer++) fprintf(fp_test,"%7d %7lld\n",i_group+i_buffer,buffer_size_t[i_buffer]);
*/
     }
  }
  SID_log("Done.",SID_LOG_CLOSE);
/*
if(SID.I_am_Master) fclose(fp_test);
*/

  // Sanity check
  calc_sum_global(&buffered_count_local,&buffered_count,1,SID_INT,CALC_MODE_DEFAULT,SID.COMM_WORLD);
  if(buffered_count!=n_groups_1)
    SID_trap_error("Buffer counts don't make sense (ie %d!=%d) after writing match indices.",ERROR_LOGIC,buffered_count,n_groups_1);

  // Write match_score ...
  //    ... loop over all the groups in buffer-sized batches
  SID_log("Writing match scores...",SID_LOG_OPEN);                 
  for(i_group=0,buffered_count_local=0;i_group<n_groups_1;i_group+=n_buffer){
     // Decide this buffer iteration's size
     n_buffer=MIN(n_buffer_max,n_groups_1-i_group);
     // Set the buffer to a default value smaller than the smallest possible data size
     for(i_buffer=0;i_buffer<n_buffer;i_buffer++)
        buffer_float[i_buffer]=-1.; // Min value of match_score is 0.
     // Determine if any of the local data is being used for this buffer
     for(j_group=0;j_group<n_groups_1_local;j_group++){
        index_test=file_index_1[j_group]-i_group;
        // ... if so, set the appropriate buffer value
        if(index_test>=0 && index_test<n_buffer){
          buffer_float[index_test]=match_score[j_group];
          buffered_count_local++;
        }
     }
     // Doing a global max on the buffer yields the needed buffer on all ranks
     SID_Allreduce(SID_IN_PLACE,buffer_float,n_buffer,SID_FLOAT,SID_MAX,SID.COMM_WORLD);
     // Sanity check
     for(i_buffer=0;i_buffer<n_buffer;i_buffer++){
       if(buffer_float[i_buffer]<-0.)
         SID_trap_error("Illegal match_score result (%f) for group No. %d.",ERROR_LOGIC,buffer_float[i_buffer],i_group+i_buffer);
     }
     // Write the buffer
     if(SID.I_am_Master)
       fwrite(buffer,sizeof(float),(size_t)n_buffer,fp_out);
  }
  SID_log("Done.",SID_LOG_CLOSE);

  // Sanity check
  calc_sum_global(&buffered_count_local,&buffered_count,1,SID_INT,CALC_MODE_DEFAULT,SID.COMM_WORLD);
  if(buffered_count!=n_groups_1)
     SID_trap_error("Buffer counts don't make sense (ie %d!=%d) after writing match scores.",ERROR_LOGIC,buffered_count,n_groups_1);

  // Close the file
  if(SID.I_am_Master)
     fclose(fp_out);

  // Clean-up
  SID_free(SID_FARG buffer);
  SID_free(SID_FARG match_rank);
  SID_log("Done.",SID_LOG_CLOSE);
}

