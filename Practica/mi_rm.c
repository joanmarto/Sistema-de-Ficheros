//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis no válida: ./mi_rm <nombre disco> </ruta> \n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_rm.c --> Error: bmount()\n");
#endif
        return -1;
    }
    int error = mi_unlink(argv[2]);
    if (error < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_rm.c --> Error en mi_unlink()\n");
#endif
        return -1;
    }
    if ((bumount()) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_rm.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}