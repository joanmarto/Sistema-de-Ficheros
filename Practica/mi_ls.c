//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis no válida: ./mi_ls <nombre disco> </ruta> \n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_ls.c -> Error: bmount()\n");
#endif
        return -1;
    }

    char buffer[TAMBUFFER];
    int error;
    memset(buffer, '\0', TAMBUFFER);
    if ((error = mi_dir(argv[2], buffer)) < 0) //Leemos todas las entradas del directorio
    {
        mostrar_error_buscar_entrada(error);
        return -1;
    }
    fprintf(stderr, "Nombre\t\tModo\tmTime\t\t\tTamaño\tTipo\n");
    fprintf(stderr, "--------------------------------------------------------------------------\n");
    fprintf(stderr, "%s\n", buffer);
    if ((bumount()) < 0)
    {
#if DEBUG8
        fprintf(stderr, "mi_ls.c --> Error: bumount()\n");
#endif
        return -1;
    }
    return 0;
}