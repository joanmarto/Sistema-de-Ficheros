//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "directorios.h"
#include "string.h"
static struct UltimaEntrada UltimaEntradaEscritura;
static struct UltimaEntrada UltimaEntradaLectura;

int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    if (camino[0] == '/') //ha de empezar por /
    {
        char *new_camino = malloc(strlen(camino));
        //int longitud = strlen(camino);
        //char *new_camino = malloc(longitud);
        //strcpy(new_camino, camino);
        new_camino = strchr(camino + 1, '/');
        memset(inicial, '\0', strlen(inicial));
        memset(final, '\0', strlen(final));
        if (new_camino != NULL)
        {
            int pos = 1;
            while (camino[pos] != '/')
            {
                pos++;
            }

            *tipo = 'd'; //tipo directorio
            strncpy(inicial, camino + 1, pos-1);
            inicial[pos-1]='\0';
            //fprintf(stderr,"inicial: %s\n", inicial);
            strcpy(final, new_camino);
        }
        else
        {
            strcpy(inicial, camino + 1);
            *tipo = 'f'; //tipo fichero
            final = "";
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    //Variables
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    memset(inicial, '\0', strlen(inicial));
    memset(final, '\0', strlen(final));
    char tipo;
    unsigned int cant_entradas_inodo, num_entrada_inodo;

    if (strcmp(camino_parcial, "/") == 0)
    {
        *p_inodo = SB.posInodoRaiz; //La raiz siempre está en el inodo 0
        *p_entrada = 0;
        return 0;
    }
    if (extraer_camino(camino_parcial, inicial, final, &tipo) < 0)
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return ERROR_CAMINO_INCORRECTO;
    }
    else
    {
#if DEBUG7
        fprintf(stderr, "[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif
    }
    //Buscamos la entrada cuyo nombre se encuentre en inicial
    if (leer_inodo(*p_inodo_dir, &inodo_dir) < 0)
    {
        fprintf(stderr, "buscar_entrada(): Error leer_inodo\n");
        return -1;
    }
    if ((inodo_dir.permisos & 4) != 4) //if ((inodo_dir.permisos & 4) != 4)       //editado por pau dia 11
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return ERROR_PERMISO_LECTURA;
    }

    //El buffer de lectura puede ser un struct de tipo entrada
    //o bien un array de las entradas que caben en un bloque, para optimizar la lectura en RAM
    struct entrada entradas[BLOCKSIZE / sizeof(entrada)];
    memset(entradas, '\0', BLOCKSIZE);
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(entrada);
    struct inodo inodo;
    int offset = 0;
    num_entrada_inodo = 0; //nº de entrada inicial
    if (cant_entradas_inodo > 0)
    {
        //leer entrada
        if (mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE) < 0)
        {
#if DEBUG7
            fprintf(stderr, "buscar_entrada(): Error al llamar a mi_reaf_f()\n");
#endif
            return -1;
        }
        entrada = entradas[0];
        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entrada.nombre) != 0))
        {
            num_entrada_inodo++;
            //leer siguente entrada
            if ((num_entrada_inodo % (BLOCKSIZE / sizeof(entrada))) == 0)
            {
                //Llenamos el buffer de 0's
                memset(entradas, '\0', BLOCKSIZE);
                offset += BLOCKSIZE;
                if (mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE) < 0)
                {
#if DEBUG7
                    fprintf(stderr, "buscar_entrada(): Error al llamar a mi_reaf_f()\n");
#endif
                    return -1;
                }
            }
            entrada = entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(entrada))];
                
        }
    }
    if ((strcmp(inicial, entrada.nombre) != 0)) //La entrada no existe
    {
        switch (reservar)
        {
        case 0: //Modo consulta
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
        case 1: //Modo escritura
            if (inodo_dir.tipo == 'f')
            {
                mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }

            //Si es directorio comprobar que tiene permisos de escritura
            if ((inodo_dir.permisos & 2) != 2) // editado por pau dia 11, i //if ((inodo_dir.permisos & 2) != 2)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_ESCRITURA);
                return ERROR_PERMISO_ESCRITURA;
            }
            else
            {
                strcpy(entrada.nombre, inicial);
                if (tipo == 'd')
                {
                    if (strcmp(final, "/") == 0)
                    {
                        entrada.ninodo = reservar_inodo('d', permisos);
                        
                        if (leer_inodo(entrada.ninodo, &inodo) < 0)
                        {
#if DEBUG7
                            fprintf(stderr, "buscar_entrada(): Error al leer inodo\n");
#endif
                            return -1;
                        }
#if DEBUG7
                        fprintf(stderr, "[buscar_entrada()→ reservado inodo %d, tipo %c, permisos %d, para: %s]\n", entrada.ninodo, tipo, inodo.permisos, entrada.nombre);
#endif
                    }
                    else //Cuelgan más directorios o ficheros
                    {
                        mostrar_error_buscar_entrada(ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO);
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                }
                else //Es un fichero
                {
                    entrada.ninodo = reservar_inodo('f', permisos);
                    
                    if (entrada.ninodo < 0)
                    {
#if DEBUG7
                        fprintf(stderr, "buscar_entrada(): Error al llamar a reservar_inodo()\n");
#endif
                        return -1;
                    }
                    if (leer_inodo(entrada.ninodo, &inodo) < 0)
                    {
#if DEBUG7
                        fprintf(stderr, "buscar_entrada(): Error al llamar a leer_inodo()\n");
#endif
                        return -1;
                    }
#if DEBUG7
                    fprintf(stderr, "[buscar_entrada()→ reservado inodo %d, tipo %c, permisos %d, para: %s]\n", entrada.ninodo, tipo, inodo.permisos, entrada.nombre);
#endif
                }
                entradas[num_entrada_inodo % (BLOCKSIZE / sizeof(entrada))] = entrada;
                if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) < 0)
                {
#if DEBUG7
                    fprintf(stderr, "buscar_entrada(): Error al llamar a mi_write_f()\n");
#endif
                    if (entrada.ninodo != -1)
                    {
                        liberar_inodo(entrada.ninodo);
                    }
                    return -1;
                }
                else
                {
#if DEBUG7
                    fprintf(stderr, "[buscar_entrada()→ creada entrada: %s, %d]\n", inicial, entrada.ninodo);
#endif
                }
            }
        }
    }
    if ((strcmp(final, "/") == 0) || (strcmp(final, "") == 0)) //Hemos llegado al final del camino: final == '/' || final == ""
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            //Modo escritura y la entrada ya existe
            mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        //Cortamos la recursividad
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;

        return 0;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return 0;
}

void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {

    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}

int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    int error_encontrado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos);
    if (error_encontrado < 0)
    {
        //Otros posibles errores que devolvería al usuario a través de buscar_entrada():
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}


int mi_dir(const char *camino, char *buffer)
{

    int numentradas = 0;
    unsigned int offset = 0, leidos = 0;
    unsigned int p_inodo = 0, p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    struct inodo inodo;
    struct entrada entrada;
    struct tm *tm;

    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);
    if (error < 0)
    {
        return error;
    }

    if (leer_inodo(p_inodo, &inodo) < 0)
    {
#if DEBUG8
        fprintf(stderr, "(i_dir): Error al llamar a leer_inodo)\n");
#endif
        return -1;
    }

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "EL fichero %s no tiene permisos de lectura\n", camino);
        return -1;
    }
    if (inodo.tipo != 'd')
    {
        fprintf(stderr, "EL fichero %s no es un directorio\n", camino);
        return -1;
    }
    char bufferAux[TAMBUFFER];
    struct inodo inodoAux;
    while ((leidos = mi_read_f(p_inodo, &entrada, offset, sizeof(entrada))) > 0)
    {
        memset(bufferAux, '\0', strlen(bufferAux));
        if (leer_inodo(entrada.ninodo, &inodoAux) < 0)
        {
#if DEBUG8
            fprintf(stderr, "mi_dir(): Error al llamar a leer_inodo()\n");
#endif
            return -1;
        }
        strcat(bufferAux, entrada.nombre);
        strcat(bufferAux, "\t");
        if (inodoAux.permisos & 4)
        {
            strcat(bufferAux, " r");
        }
        else
        {

            strcat(bufferAux, "-");
        }
        if (inodoAux.permisos & 2)
        {
            strcat(bufferAux, "w");
        }
        else
        {

            strcat(bufferAux, "-");
        }
        if (inodoAux.permisos & 1)
        {
            strcat(bufferAux, "x");
        }
        else
        {
            strcat(bufferAux, "-");
        }
        strcat(bufferAux, "\t");
        char tmp[50];
        char tamEnBytesLog[10];
        char tipo[2];
        tm = localtime(&inodoAux.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(bufferAux, tmp);
        strcat(bufferAux, "\t");
        sprintf(tamEnBytesLog, "%d", inodoAux.tamEnBytesLog);
        strcat(bufferAux, tamEnBytesLog);
        strcat(bufferAux, "\t");
        sprintf(tipo, "%c", inodoAux.tipo);
        strcat(bufferAux, tipo);
        strcat(bufferAux, "\n"); //Separador
        strcat(buffer, bufferAux);
        offset += leidos;
        numentradas++;
    }
    return numentradas;
}

int mi_chmod(const char *camino, unsigned char permisos)
{
    unsigned int p_inodo = 0, p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    int error_encontrado = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos);
    if (error_encontrado == 0)
    {
        mi_chmod_f(p_inodo, permisos);
        return 0;
    }
    else
    {
        return -1;
    }
}

int mi_stat(const char *camino, struct STAT *p_stat)
{
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    int error_buscar_entrada = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4);

    if (error_buscar_entrada == 0)
    {
        //Si la entrada existe entra aqui
        int error_mi_stat = mi_stat_f(p_inodo, p_stat);
        if (error_mi_stat < 0)
        {
            //lamamos a la función correspondiente de ficheros.c pasándole el p_inodo, y mostramos su p_inodo
            fprintf(stderr, "mi_stat(): Error, el p_inodo es: %d \n", p_inodo);
            return -1;
        }
        return p_inodo;
    }
    else
    {
        return -1;
    }
}

//nivel 9
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{

    unsigned int p_inodo = 0, p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    int bytesescritos = 0;
    //si entra, es una escritura sobre el mismo inodo
    if (strcmp(UltimaEntradaEscritura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaEscritura.p_inodo;
        bytesescritos = mi_write_f(p_inodo, buf, offset, nbytes);
        if (bytesescritos < 0)
        {
#if DEBUG9
            fprintf(stderr, "mi_write(): Error al llamar a mi_write_f()\n");
#endif
            return -1;
        }
    }
    else
    {
        buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6); //110
        strcpy(UltimaEntradaEscritura.camino, camino);
        UltimaEntradaEscritura.p_inodo = p_inodo;
        bytesescritos = mi_write_f(p_inodo, buf, offset, nbytes);
        if (bytesescritos < 0)
        {
#if DEBUG9
            fprintf(stderr, "mi_write(): Error al llamar a mi_write_f()\n");
#endif
            return -1;
        }
    }

    return bytesescritos;
}

int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{

    unsigned int p_inodo = 0, p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    int bytesleidos = 0;
    if (strcmp(UltimaEntradaLectura.camino, camino) == 0)
    {
        p_inodo = UltimaEntradaLectura.p_inodo;
        bytesleidos = mi_read_f(p_inodo, buf, offset, nbytes);
        if (bytesleidos < 0)
        {
#if DEBUG9
            fprintf(stderr, "mi_read(): Error al llamar a mi_read_f()\n");
#endif
            return -1;
        }
    }
    else
    {
        buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4); //100
        strcpy(UltimaEntradaLectura.camino, camino);
        UltimaEntradaLectura.p_inodo = p_inodo;
        bytesleidos = mi_read_f(p_inodo, buf, offset, nbytes);
        if (bytesleidos < 0)
        {
#if DEBUG9
            fprintf(stderr, "mi_read(): Error al llamar a mi_read_f()\n");
#endif
            return -1;
        }
    }

    return bytesleidos;
}

int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem();
    unsigned int p_inodo = 0, p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    struct entrada entrada;
    struct inodo inodo;
    unsigned int p_inodo1;
    int err;
    if ((err = buscar_entrada(camino1, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6)) < 0)
    {
        mi_signalSem();
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link(): Error al llamar a leer_inodo()\n");
#endif
        mi_signalSem();
        return -1;
    }
    p_inodo1 = p_inodo;
    if (inodo.tipo != 'f')
    {
        fprintf(stderr, "\nEl camino no es un fichero\n");
        mi_signalSem();
        return -1;
    }
    if ((inodo.permisos & 4) != 4)
    {
        fprintf(stderr, "\nEl fichero no tiene permisos de lectura\n");
        mi_signalSem();
        return -1;
    }

    p_inodo = 0, p_entrada = 0;
    p_inodo_dir = 0;
    if ((err = buscar_entrada(camino2, &p_inodo_dir, &p_inodo, &p_entrada, 1, 6)) < 0)
    {
        mi_signalSem();
        return -1;
    }

    if (mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link(): Error al llamar a mi_read_f()\n");
#endif
        mi_signalSem();
        return -1;
    }
    entrada.ninodo = p_inodo1;
    if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link(): Error al llamar a mi_write_f()\n");
#endif
        mi_signalSem();
        return -1;
    }
    if (liberar_inodo(p_inodo) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link(): Error al llamar a liberar_inodo()\n");
#endif
        mi_signalSem();
        return -1;
    }
    if (leer_inodo(p_inodo1, &inodo) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link(): Error al llamar a leer_inodo()\n");
#endif
        mi_signalSem();
        return -1;
    }
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    if (escribir_inodo(p_inodo1, inodo) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_link(): Error al llamar a escribir_inodo()\n");
#endif
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}
int mi_unlink(const char *camino)
{
    mi_waitSem();
    unsigned int p_inodo = 0, p_entrada = 0;
    unsigned int p_inodo_dir = 0;
    struct entrada entrada;
    struct inodo inodo;
    int nentradas = 0;
    int err = 0;
    int liberados = 0;
    if ((err = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6)) < 0)
    {
        mi_signalSem();
        return -1;
    }
    if ((leer_inodo(p_inodo, &inodo)) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_unlink(): Error al llamar a leer_inodo()\n");
#endif

        mi_signalSem();
        return -1;
    }

    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0))
    {
        fprintf(stderr, "El directorio no esta vacio\n");
        mi_signalSem();
        return -1;
    }

    if ((leer_inodo(p_inodo_dir, &inodo)) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_unlink(): Error al llamar a leer_inodo()\n");
#endif
        mi_signalSem();
        return -1;
    }
    nentradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    if (nentradas - 1 != p_entrada)
    {
        if (mi_read_f(p_inodo_dir, &entrada, (nentradas - 1) * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
#if DEBUG10
            fprintf(stderr, "mi_link(): Error al llamar a mi_read_f()\n");
#endif
            mi_signalSem();
            return -1;
        }
        if (mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada)) < 0)
        {
#if DEBUG10
            fprintf(stderr, "mi_unlink(): Error al llamar a mi_write_f()\n");
#endif
            mi_signalSem();
            return -1;
        }
    }
    liberados = mi_truncar_f(p_inodo_dir, (nentradas - 1) * sizeof(struct entrada));
    if (liberados < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_unlink(): Error al llamar a mi_truncar_f()\n");
#endif
        mi_signalSem();
        return -1;
    }
    if (leer_inodo(p_inodo, &inodo) < 0)
    {
#if DEBUG10
        fprintf(stderr, "mi_unlink(): Error al llamar a leer_inodo()\n");
#endif
        mi_signalSem();
        return -1;
    }
    inodo.nlinks--;
    if (inodo.nlinks == 0)
    {
        if (liberar_inodo(p_inodo) < 0)
        {
#if DEBUG10
            fprintf(stderr, "mi_unlink(): Error al llamar a liberar_inodo()\n");
#endif
            mi_signalSem();
            return -1;
        }
    }
    else
    {
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, inodo) < 0)
        {
#if DEBUG10
            fprintf(stderr, "mi_unlink(): Error al llamar a escribir_inodo()\n");
#endif
            mi_signalSem();
            return -1;
        }
    }
    fprintf(stderr, "\nLiberados: %d\n", liberados);
    mi_signalSem();
    return 0;
}