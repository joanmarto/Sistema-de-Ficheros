//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros.h"

int main(int argc, char **argv)
{
    if (argc == 4)
    {
        if (bmount(argv[1]) != -1)
        {
            struct STAT stat;
            int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
            int longitud = strlen(argv[2]);
#if DEBUG5
            fprintf(stderr, "Longitud del texto: %d \n", longitud);
#endif
            unsigned int ninodo = reservar_inodo('f', 6);
            if (ninodo < 0)
            {
#if DEBUG5
                fprintf(stderr, "escribir.c -> Error al llamar a reservar_inodo()\n");
#endif
                return -1;
            }
            for (int i = 0; i < 5; i++)
            {
#if DEBUG5
                fprintf(stderr, "Nº inodo reservado: %d \n", ninodo);
                fprintf(stderr, "Offset: %d \n", offsets[i]);
#endif
                int byte_escritos = mi_write_f(ninodo, argv[2], offsets[i], longitud);
#if DEBUG5
                fprintf(stderr, "Bytes escritos: %d \n", byte_escritos);
#endif
                if (byte_escritos < 0)
                {
#if DEBUG5
                    fprintf(stderr, "escribir.c -> Error al llamar a mi_write_f()\n");
#endif
                    return -1;
                }
                if (mi_stat_f(ninodo, &stat) < 0)
                {
#if DEBUG5
                    fprintf(stderr, "escribir.c -> Error al llamar a mi_stat_f()\n");
#endif
                    return -1;
                }
#if DEBUG5
                fprintf(stderr, "stat.tamEnBytesLog = %d \n", stat.tamEnBytesLog);
                fprintf(stderr, "stat.numBloquesOcupados : %d", stat.numBloquesOcupados);
                fprintf(stderr, "\n\n\n");
#endif
                if (atoi(argv[3]) > 0) //Cada offset va en un inodo distinto
                {
                    ninodo = reservar_inodo('f', 6);
                    if (ninodo < 0)
                    {
#if DEBUG5
                        fprintf(stderr, "escribir.c -> Error al llamar a reservar_inodo()\n");
#endif
                        return -1;
                    }
                }
            }
            if (bumount() < 0)
            {
#if DEBUG5
                fprintf(stderr, "escribir.c -> Error al desmontar el dispositivo\n");
#endif
                return -1;
            }
        }
        else
        {
#if DEBUG5
            fprintf(stderr, "escribir.c -> Error al montar el dispositivo\n");
#endif
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Sintaxis incorecta: ./escribir <nombre disco> <texto> <diferentes_inodos>\n");
        return -1;
    }
    return 0;
}