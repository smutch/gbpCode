#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gbpLib.h>
#include <gbpMath.h>
#include <gbpHalos.h>
#include <gbpTrees_build.h>

int check_goodness_of_match(int n_particles,float match_score){
   if(match_score<maximum_match_score(READ_MATCHES_GOODNESS_FS*(double)n_particles))
      return(FALSE);
   else
      return(TRUE);
}

