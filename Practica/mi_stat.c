//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis no válida: ./mi_stat <disco> <ruta> \n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_stat.c --> Error: bmount()\n");
#endif
        return -1;
    }
    const char *camino = argv[2];
    struct STAT stat;
    int ninodo = mi_stat(camino, &stat);
    if (ninodo < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_stat.c --> Error en mi_stat() \n");
#endif
    }
    else
    {
        fprintf(stderr, "Nº de inodo: %d \n", ninodo);
        fprintf(stderr, "Fichero: %s\n", (argv[1]));
        fprintf(stderr, "tamEnBytesLog: %u\n", stat.tamEnBytesLog);
        fprintf(stderr, "numBloquesOcupados: %u\n", stat.numBloquesOcupados);
        fprintf(stderr, "Bloques E/S: %d \n", BLOCKSIZE);
        fprintf(stderr, "Tipo: %c\n", stat.tipo);
        fprintf(stderr, "Permisos: %d\n", stat.permisos);
        fprintf(stderr, "atime: %s", ctime(&stat.atime));
        fprintf(stderr, "mtime: %s", ctime(&stat.mtime));
        fprintf(stderr, "ctime: %s", ctime(&stat.ctime));
        fprintf(stderr, "nlinks: %d \n", stat.nlinks);
    }
    if ((bumount()) < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_stat.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}