//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros.h"
int main(int argc, char **argv)
{
    if (argc == 4)
    {
        if (bmount(argv[1]) != -1)
        { //montmos el dispositivo virtual
            struct superbloque SB;
            struct STAT p_stat;
            if (bread(0, &SB) < 0) //leemos el superbloque
            {
#if DEBUG6
                fprintf(stderr, "truncar.c --> error en bread\n");
#endif
                return -1;
            }

            int ninodo = atoi(argv[2]);
            int nbytes = atoi(argv[3]);
            if (nbytes == 0)
            {
#if DEBUG6
                fprintf(stderr, "truncar.c -> cantBloquesLibres = %d \n", SB.cantBloquesLibres);
#endif
                if (liberar_inodo(ninodo) < 0)
                {
#if DEBUG6
                    fprintf(stderr, "truncar.c --> Error en librerar_inodo() \n");
#endif
                    return -1;
                }
                if (bread(0, &SB) < 0)
                {
#if DEBUG6
                    fprintf(stderr, "truncar.c --> error en bread\n");
#endif
                    return -1;
                }
#if DEBUG6
                fprintf(stderr, "truncar.c -> cantBloquesLibres (actualizado) = %d \n", SB.cantBloquesLibres);
                fprintf(stderr, "truncar.c -> cantInodosLibres (actualizado) = %d \n", SB.cantInodosLibres);
#endif
            }
            else
            {
                mi_truncar_f(ninodo, nbytes);
            }
            if (mi_stat_f(ninodo, &p_stat) < 0)
            {
#if DEBUG6
                fprintf(stderr, "truncar.c --> Error: mi_stat_f\n")
#endif
                    return -1;
            }
            fprintf(stderr, "\ntruncar.c -> tamEnBytesLog = %d \n", p_stat.tamEnBytesLog);
            fprintf(stderr, "truncar.c -> numBloquesOcupados = %d \n", p_stat.numBloquesOcupados);
            if ((bumount()) < 0) //desmontamos el dispositivo virtual
            {
#if DEBUG6
                fprintf(stderr, "truncar.c ->  Error: bumount()\n");
#endif
                return -1;
            }
        }
    }
    else
    {
        fprintf(stderr,"Sintaxis incorecta: ./truncar <nombre disco> <ninodo> <nbytes>\n");
    }
}
