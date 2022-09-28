//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "ficheros.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Sintaxis no válida:  ./leer.c <nombre_dispositivo> <ninodo>\n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG5
        fprintf(stderr, "leer.c -> Error al llamar a bmount()\n");
#endif
        return -1;
    }                                    //le pasamos el nombre del disco
    unsigned int ninodo = atoi(argv[2]); //le pasamos el ninodo
    int leidos = 0;
    int BytesLeidos = 0;
    int offset = 0;
    int tambuffer = 1500;
    void *buf_original[tambuffer];
    struct STAT stat;

    memset(buf_original, '\0', tambuffer);
    if ((leidos = mi_read_f(ninodo, buf_original, offset, tambuffer)) < 0)
    {
#if DEBUG5
        fprintf(stderr, "leer.c --> Error: mi_read_f()\n");
#endif
        return -1;
    }
    offset += leidos;
    BytesLeidos += leidos;
    while (leidos > 0)
    {
        write(1, buf_original, leidos);
        memset(buf_original, '\0', tambuffer);
        if ((leidos = mi_read_f(ninodo, buf_original, offset, tambuffer)) < 0)
        {
#if DEBUG5
            fprintf(stderr, "leer.c --> Error: mi_read_f()\n");
#endif
            return -1;
        }
        BytesLeidos += leidos;
        offset += leidos;
    }
    fprintf(stderr, "\nBytes leídos: %d\n", BytesLeidos);

    if ((mi_stat_f(ninodo, &stat)) < 0)
    {
#if DEBUG5
        fprintf(stderr, "leer.c --> Error: mi_stat_f\n");
#endif
        return -1;
    }
    fprintf(stderr, "Tamaño en bytes logicos: %d\n", stat.tamEnBytesLog);

    if ((bumount()) < 0)
    {
#if DEBUG5
        fprintf(stderr, "leer.c --> Error: bumount()");
#endif
        return -1;
    }
    return 0;
}