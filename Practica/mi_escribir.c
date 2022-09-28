//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "\nSintaxis no válida: ./mi_escribir <disco> </ruta_fichero> <texto> <offset> \n");
        return -1;
    }

    if ((bmount(argv[1])) < 0)
    {
#if DEBUG9
        fprintf(stderr, "mi_escribir.c --> Error: bmount()\n");
#endif
        return -1;
    }

    char *camino = argv[2];
    char *texto = argv[3];
    unsigned int offset = atoi(argv[4]);

    fprintf(stderr, "mi_escribir.c -> camino : %s\n", camino);
    fprintf(stderr, "mi_escribir.c -> texto : %s\n", texto);
    fprintf(stderr, "mi_escribir.c -> offset : %d\n", offset);

    int escritura = mi_write(camino, texto, offset, strlen(texto));
    if (escritura < 0)
    {
#if DEBUG9
        fprintf(stderr, "Error en mi_write()\n");
#endif
        return -1;
    }
    fprintf(stderr, "\nNúmero de bytes escritos: %d\n", escritura);
    if ((bumount()) < 0)
    {
#if DEBUG9
        fprintf(stderr, "mi_escribir.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}