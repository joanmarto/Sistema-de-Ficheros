//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
//Programa para testear las cachés de directorios
#include "directorios.h"

int main(int argc, char **argv)
{

  //Comprobamos sintaxis
  if (argc != 5)
  {
    fprintf(stderr, "Sintaxis: mi_escribir <nombre_dispositivo> </ruta_fichero> <texto> <offset>\n");
    exit(-1);
  }

  //struct STAT stat;

  //montamos el dispositivo
  if ((bmount(argv[1])) < 0)
  {
#if DEBUG9
    fprintf(stderr, "mi_escribir_varios.c --> Error: bmount()\n");
#endif
    return -1;
  }
  //obtenemos el texto y su longitud
  char *buffer_texto = argv[3];
  int longitud = strlen(buffer_texto);

  //obtenemos la ruta y comprobamos que no se refiera a un directorio
  if (argv[2][strlen(argv[2]) - 1] == '/')
  {
    fprintf(stderr, "mi_escribir_varios.c --> Error: la ruta se corresponde a un directorio");
    exit(-1);
  }
  char *camino = argv[2];
  //obtenemos el offset
  unsigned int offset = atoi(argv[4]);
//escribimos el texto
#if DEBUG9
  fprintf(stderr, "mi_escribir_varios.c --> camino: %s\n", camino);
  fprintf(stderr, "mi_escribir_varios.c --> buffer_texto: %s\n", buffer_texto);
  fprintf(stderr, "mi_escribir_varios.c --> offset: %d\n", offset);
#endif
  int escritos = 0;
  int varios = 10;
  fprintf(stderr, "mi_escribir_varios.c --> longitud texto: %d\n", longitud);
  for (int i = 0; i < varios; i++)
  {
    // escribimos varias veces el texto desplazado 1 bloque
    if ((escritos += mi_write(camino, argv[3], offset + BLOCKSIZE * i, longitud)) < 0)
    {
#if DEBUG9
      fprintf(stderr, "mi_escribir_varios.c --> Error: mi_write()");
#endif
      return -1;
    }
  }
  fprintf(stderr, "mi_escribir_varios.c --> Bytes escritos: %d\n", escritos);
//Visualización del stat
#if DEBUG9
  if (mi_stat_f(ninodo, &stat) < 0)
  {
    fprintf(stderr, "mi_escribir_varios.c -> Error al llamar a mi_stat()\n");
    return -1;
  }
  fprintf(stderr, "stat.tamEnBytesLog=%d\n", stat.tamEnBytesLog);
  fprintf(stderr, "stat.numBloquesOcupados=%d\n", stat.numBloquesOcupados);
#endif

  if ((bumount()) < 0)
  {
#if DEBUG9
    fprintf(stderr, "mi_escribir_varios.c --> Error: bumount()\n");
#endif
    return -1;
  }
}