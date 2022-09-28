//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis no válida: <disco> <permisos> </ruta> \n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_chmod.c --> Error: bmount()\n");
#endif
        return -1;
    }
    unsigned char permisos = (unsigned char)atoi(argv[2]);
    const char *camino = argv[3];

    if ((permisos >= 0) && (permisos < 8)) //Hay que comprobar que permisos sea un nº válido (0-7)
    {
        if ((mi_chmod(camino, permisos)) < 0)
        {
#if DEBUG8
            fprintf(stderr, "mi_chmod.c --> Error: mi_chmod()\n");
#endif
            return -1;
        }
        return 0;
    }
    else
    {
        fprintf(stderr, "Permisos fuera de rango\n");
        return -1;
    }
    if ((bumount()) < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_chmod.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}