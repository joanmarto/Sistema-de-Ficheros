//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
//Programa opcional para crear fichero (no directorios)
#include "directorios.h"

int main(int argc, char **argv)
{
  //Comprobamos sintaxis
  if (argc != 4)
  {
    fprintf(stderr, "Sintaxis: mi_touch <nombre_dispositivo> <permisos> </ruta_fichero>\n");
    exit(-1);
  }
  //montamos el dispositivo
  if (bmount(argv[1]) < 0)
  {
#if DEBUG8
    fprintf(stderr, "mi_touch.c --> Error: bmount()\n");
#endif
    return -1;
  }

  //obtenemos el texto y su longitud
  int permisos = atoi(argv[2]);
  int error = mi_creat(argv[3], permisos);
  if (error < 0)
  {
#if DEBUG8
    fprintf(stderr, "mi_touch.c --> Error en mi_creat() \n");
#endif
    return -1;
  }
  if ((bumount()) < 0)
  {
#if DEBUG8
    fprintf(stderr, "mi_touch.c --> Error: bumount()\n");
#endif
    return -1;
  }
}