//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros.h"
int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Error de sintaxis, sintaxis correcta: permitir <nombre_dispositivo> <ninodo> <permisos>\n");
    }
    unsigned char permisos = atoi(argv[3]);
    if (bmount(argv[1]) != -1)
    {
        if ((mi_chmod_f((unsigned int)atoi(argv[2]), (unsigned char)permisos)) < 0)
        {
#if DEBUG5
            fprintf(stderr, "permitir.c --> Error: mi_chmod_f()\n");
#endif
            return -1;
        }
        if ((bumount()) < 0)
        {
#if DEBUG5
            fprintf(stderr, "permitir.c --> Error: bumount()\n");
#endif
            return -1;
        }
    }
    else
    {
#if DEBUG5
        fprintf(stderr, "permitir.c -> Error al montar elo dispositivo \n");
#endif
        return -1;
    }
    return 0;
}