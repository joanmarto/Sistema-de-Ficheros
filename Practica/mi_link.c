//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis no válida: ./mi_link <nombre disco> </ruta_fichero_original> </ruta> \n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link.c --> Error: bmount()\n");
#endif
        return -1;
    }
    int error = mi_link(argv[2], argv[3]);
    if (error < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link.c -> Error al llamar a mi_link()\n");
#endif
        return -1;
    }
    if ((bumount()) < 0)
    {
#if DERBUG10
        fprintf(stderr, "mi_link.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}