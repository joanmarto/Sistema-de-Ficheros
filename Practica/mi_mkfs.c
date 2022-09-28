//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
//#include "leer_sf.c"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis no valida: ./mi_mkfs <nombre del dispositivo> <número de bloques> \n");
    }
    else
    {
        if ((bmount(argv[1])) < 0)
        {
            fprintf(stderr, "mi_mkfs.c --> Error: bmount()\n");
            return -1;
        }
        unsigned char buff[BLOCKSIZE];
        int nbloques = atoi(argv[2]);
        memset(buff, '\0', BLOCKSIZE);
        for (int i = 0; i < nbloques; i++)
        {
            if (bwrite(i, buff) < 0)
            {
#if DEBUG1
                fprintf(stderr, "mi_mkfs.c --> Error: bwrite\n");
#endif
                return -1;
            }
        }
        if (initSB(nbloques, nbloques / 4) < 0)
        {
#if DEBUG2
            fprintf(stderr, "mi_mkfs.c --> Error al llamar a initSB()\n");
#endif
            return -1;
        }
        if (initMB() < 0)
        {
#if DEBUG2
            fprintf(stderr, "mi_mkfs.c --> Error al llamar a initMB()\n");
#endif
            return -1;
        }
        if (initAI() < 0)
        {
#if DEBUG2
            fprintf(stderr, "mi_mkfs.c --> Error al llamar a initAI()\n");
#endif
            return -1;
        }
        if (reservar_inodo('d', 7) < 0)
        {
#if DEBUG2
            fprintf(stderr, "mi_mkfs.c --> Error: reservar_inodo()\n");
#endif
            return -1;
        }
        if ((bumount()) < 0)
        {
#if DEBUG1
            fprintf(stderr, "mi_mkfs.c --> Error: bumount()\n");
#endif
            return -1;
        }
    }
}
