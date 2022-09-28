//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Sintaxis no válida: ./mi_mkdir <disco> <permisos> </ruta> \n");
        return -1;
    }
    else
    {
        if ((bmount(argv[1])) < 0)
        {
#if DEBUG8
            fprintf(stderr, "mi_mkdir.c --> Error: bmount()\n");
#endif
            return -1;
        }
        int permisos = atoi(argv[2]);
        if (permisos < 0 || permisos > 7)
        {
#if DEBUG8
            fprintf(stderr, "Permiso %d no válido. Ha de estar entre 0 y 7.\n", permisos);
#endif
            return -1;
        }
        unsigned char p = (unsigned char)atoi(argv[2]);
        if (mi_creat(argv[3], p) < 0)
        {
#if DEBUG8
            fprintf(stderr, "mi_mkdir.c -> Error al llamar a mi_creat()\n");
#endif
            return -1;
        }
        if ((bumount()) < 0)
        {
#if DEBUG8
            fprintf(stderr, "mi_mkdir.c --> Error: bumount()\n");
#endif
            return -1;
        }
    }
    return 0;
}