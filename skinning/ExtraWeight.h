#ifndef EXRTAWEIGHT_H
#define EXRTAWEIGHT_H
enum ExtraWeight
{
  EXTRAWEIGHT_PROJECT_COMPACT, // smoothed projection based weights in weight space
  EXTRAWEIGHT_ISO, // isotropic bumps in weight space
}; 
#define NUM_EXTRAWEIGHT 2
#endif
