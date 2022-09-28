//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "simulacion.h"
 
struct INFORMACION {
  int pid;
  unsigned int nEscrituras; //validadas 
  struct REGISTRO PrimeraEscritura;
  struct REGISTRO UltimaEscritura;
  struct REGISTRO MenorPosicion;
  struct REGISTRO MayorPosicion;
};
