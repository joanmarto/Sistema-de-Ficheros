//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "\nSintaxis no válida: ./mi_cat <disco> </ruta_fichero> \n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG9
        fprintf(stderr, "mi_cat.c --> Error: bmount()\n");
#endif
        return -1;
    }
    char *camino = argv[2];
    int BytesLeidos = 0;
    int offset = 0;
    int tambuffer = 1500;
    int leidos = 0;
    void *buf_original[tambuffer];

    if ((camino[strlen(camino) - 1] != '/') != 0)
    {

        memset(buf_original, '\0', tambuffer);
        if ((leidos = mi_read(camino, buf_original, offset, tambuffer)) < 0)
        {
#if DEBUG9
            fprintf(stderr, "mi_cat.c --> Error: mi_read()");
#endif
            return -1;
        }
        offset += leidos;
        BytesLeidos += leidos;
        while (leidos > 0)
        {
            write(1, buf_original, leidos);
            memset(buf_original, '\0', tambuffer);

            if ((leidos = mi_read(camino, buf_original, offset, tambuffer)) < 0)
            {
#if DEBUG9
                fprintf(stderr, "mi_cat.c --> Error: mi_read()");
#endif
                return -1;
            }

            BytesLeidos += leidos;
            offset += leidos;
        }
    }
    else
    {
#if DEBUG9
        fprintf(stderr, "\nError: mi_cat.c --> No se puede hacer el cat de un directorio\n");
#endif
    }
    fprintf(stderr, "\nNúmero de bytes leidos: %d\n", BytesLeidos);

    if ((bumount()) < 0)
    {
#if DEBUG9
        fprintf(stderr, "mi_cat.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}