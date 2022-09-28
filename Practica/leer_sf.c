//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"
#include "directorios.h"

void mostrar_buscar_entrada(char *camino, char reservar); //si no se declara F en la ejecución

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Sintaxis no válida:  ./leer_sf <nombre_dispositivo>\n");
        return -1;
    }
    if ((bmount(argv[1])) < 0)
    {
#if DEBUG2
        fprintf(stderr, "leer_sf.c -> Error: al llamar a bmount()\n");
#endif
        return -1;
    }

    struct superbloque aux;
    if (bread(0, &aux) < 0) //Cargamos el bloque leido en un struct superbloque
    {
#if DEBUG2
        fprintf(stderr, "leer_sf.c -> Error: al llamar a bread()\n");
#endif
        return -1;
    }

    fprintf(stderr, "DATOSº SUPERBLOQUE: \n\n");
    fprintf(stderr, "posPrimerBloqueMB = %u\n", aux.posPrimerBloqueMB);
    fprintf(stderr, "posUltimoBloqueMB = %u\n", aux.posUltimoBloqueMB);
    fprintf(stderr, "posPrimerBloqueAI = %u\n", aux.posPrimerBloqueAI);
    fprintf(stderr, "posUltimoBloqueAI = %u\n", aux.posUltimoBloqueAI);
    fprintf(stderr, "posPrimerBloqueDatos = %u\n", aux.posPrimerBloqueDatos);
    fprintf(stderr, "posUltimoBloqueDatos = %u\n", aux.posUltimoBloqueDatos);
    fprintf(stderr, "posInodoRaiz = %u\n", aux.posInodoRaiz);
    fprintf(stderr, "posPrimerInodoLibre = %u\n", aux.posPrimerInodoLibre);
    fprintf(stderr, "cantBloquesLibres = %u\n", aux.cantBloquesLibres);
    fprintf(stderr, "cantInodosLibres = %u\n", aux.cantInodosLibres);
    fprintf(stderr, "totBloques = %u\n", aux.totBloques);
    fprintf(stderr, "totInodos = %u\n", aux.totInodos);

    fprintf(stderr, "\nTamaño superbloque = %d\n", tamSB);
    fprintf(stderr, "Tamaño inodo = %d\n", INODOSIZE);

    if ((bumount()) < 0)
    {
#if DEBUG2
        fprintf(stderr, "leer_sf.c -> Error al llamar a bumount()\n");
#endif
        return -1;
    }
    return 0;
}

void mostrar_buscar_entrada(char *camino, char reservar)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    fprintf(stderr, "\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error);
    }
    fprintf(stderr, "**********************************************************************\n");
    return;
}
