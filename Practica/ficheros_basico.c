//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros_basico.h"

//un bloque ocupa 1024 bytes
//calcula el tamaño en bloques necesario para el mapa de bits
int tamMB(unsigned int nbloques)
{
    if (((nbloques / 8) % BLOCKSIZE) == 0)
    {
        return ((nbloques / 8) / BLOCKSIZE);
    }
    else
    {
        return ((int)(nbloques / 8) / BLOCKSIZE) + 1;
    }
}

//un inodo ocupa 128 bytes
//calcula el tamaño en bloques del array de inodos
int tamAI(unsigned int ninodos)
{
    if (((ninodos * INODOSIZE) % BLOCKSIZE) == 0)
    {
        return ((int)(ninodos * INODOSIZE) / BLOCKSIZE);
    }
    else
    {
        return ((int)(ninodos * INODOSIZE) / BLOCKSIZE + 1);
    }
}

//inicializa los datos del "superbloque". Se define una variable del tipo superbloque que se va rellenando
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    //struct superbloque buff[BLOCKSIZE/];
    //struct superbloque aux;
    //memset(buff, '\0', BLOCKSIZE * sizeof(unsigned int));
    if (nbloques > 0)
    {
        SB.posPrimerBloqueMB = posSB + tamSB;
        SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
        SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
        SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
        SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
        SB.posUltimoBloqueDatos = nbloques - 1;
        SB.posInodoRaiz = 0;
        SB.posPrimerInodoLibre = 0;
        SB.cantBloquesLibres = nbloques - SB.posPrimerBloqueDatos;
        SB.cantInodosLibres = ninodos;
        SB.totBloques = nbloques;
        SB.totInodos = ninodos;
        if (bwrite(0, &SB) < 0)
        {
#if DEBUG2
            fprintf(stderr, "initSB(): Error al inicializar el SuperBloque \n");
#endif
            return -1;
        }
        //memset(buff, '\0', BLOCKSIZE * sizeof(unsigned int));
        return 0;
    }
#if DEBUG2
    fprintf(stderr, "initSB(): nbloques < 0\n");
#endif

    return -1;
}

//Inicializa con 1s todas las posiciones de SB, MB y AI, lo demas a 0
int initMB()
{
    //Buffers de escritura inicializados a cero
    unsigned char buff[BLOCKSIZE];
    unsigned char buffMB[BLOCKSIZE];
    memset(buff, '\0', BLOCKSIZE);
    memset(buffMB, '\0', BLOCKSIZE);
    //Número de bits para los metadatos
    int MBbits = 1 + tamAI(SB.totInodos) + tamMB(SB.totBloques);
    //Bits resto
    int MBbitsMod = MBbits % 8;
    //Número de bloques extra necesarios para el mapa de bits por si 1 no fuera sufuciente
    int MBbloques = (MBbits / 8) / BLOCKSIZE;
    unsigned char mascara = 128;
    //Escribe ceros en el mapa de bits
    for (int i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++)
    {
        if ((bwrite(i, buff)) < 0)
        {
#if DEBUG2
            fprintf(stderr, "initMB(): Error al llamar a bwrite() \n");
#endif
            return -1;
        }
    }
    //Llenamos de unos los bloques que son todo unos, para el caso en que los metadatos
    //ocupen más de un bloque en el mapa de bits
    if (MBbloques > 0)
    {
        memset(buffMB, 255, BLOCKSIZE);
        for (int i = 0; i < MBbloques; i++)
        {
            if ((bwrite(SB.posPrimerBloqueMB + i, buffMB)) < 0)
            {
#if DEBUG2
                fprintf(stderr, "initMB(): Error al llamar a bwrite() \n");
#endif
                return -1;
            }
        }
        memset(buffMB, '\0', BLOCKSIZE);
    }

    //Añadimos unos en los bits que representan bloques de metadatos
    for (int a = 0; a < MBbits / 8; a++)
    {
        buffMB[a] = 255;
    }
    //Añadimos unos a los bits restantes

    for (int b = 0; b < MBbitsMod; b++)
    {
        buffMB[MBbits / 8] |= mascara; //OR
        mascara >>= 1;
    }

    //Salvamos el mapa de bits
    if ((bwrite(SB.posPrimerBloqueMB + MBbloques, buffMB)) < 0)
    {
#if DEBUG2
        fprintf(stderr, "initMB(): Error al llamar a bwrite() \n");
#endif
        return -1;
    }
    memset(buffMB, '\0', BLOCKSIZE);
    memset(buff, '\0', BLOCKSIZE);
    return 0;
}

//inicializa la lsita de inodos libres
int initAI()
{
    struct superbloque SB;
    if (bread(0, &SB) < 0)
    {
#if DEBUG2
        fprintf(stderr, "initAI(): Error al llamar a bread() \n");
#endif
        return -1;
    }
    unsigned int contInodos = SB.posPrimerInodoLibre + 1;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    memset(inodos, '\0', BLOCKSIZE);
    for (size_t i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        for (size_t j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            inodos[j].tipo = 'l'; //indica que el inodo está libre
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
            }
        }
        if (bwrite(i, inodos) < 0)
        {
#if DEBUG2
            fprintf(stderr, "initAI(): Error al llamar a bwrite()\n");
#endif
            return -1;
        }
        memset(inodos, '\0', BLOCKSIZE);
    }
    if (bwrite(0, &SB) < 0)
    {
#if DEBUG2
        fprintf(stderr, "initAI(): Error al llamar a bwrite()\n");
#endif
        return -1;
    }
    return 0;
}

int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque aux;
    if (bread(0, &aux) < 0)
    {
#if DEBUG3
        fprintf(stderr, "escribir_bit(): Error al llamar a bread()");
#endif
        return -1;
    }
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = aux.posPrimerBloqueMB + nbloqueMB;
    unsigned char buffMB[BLOCKSIZE];

    if (bread(nbloqueabs, buffMB) < 0)
    {
#if DEBUG3
        fprintf(stderr, "escribir_bit(): Error al llamar a bread()\n");
#endif
        return -1;
    }

    posbyte = posbyte % BLOCKSIZE;
    unsigned char mascara = 128;
    mascara >>= posbit;
    if (bit == 1)
    {
        buffMB[posbyte] |= mascara;
    }
    else if (bit == 0)
    {
        buffMB[posbyte] &= ~mascara;
    }
    else
    {
        return -1;
    }
    if (bwrite(nbloqueabs, buffMB) < 0)
    {
#if DEBUG3
        fprintf(stderr, "escribir_bit(): Error al llamar a bwrite()\n");
#endif
        return -1;
    }
    memset(buffMB, '\0', BLOCKSIZE);
    return 0;
}

char leer_bit(unsigned int nbloque)
{
    struct superbloque aux;
    if (bread(0, &aux) < 0)
    {
#if DEBUG3
        fprintf(stderr, "leer_bit(): Error al llamar a bread()\n");
#endif
        return -1;
    }
    unsigned int posbyte = nbloque / 8;
    unsigned int posbit = nbloque % 8;
    unsigned int nbloqueMB = posbyte / BLOCKSIZE;
    unsigned int nbloqueabs = aux.posPrimerBloqueMB + nbloqueMB;
    unsigned char buffMB[BLOCKSIZE];

    if (bread(nbloqueabs, buffMB) < 0)
    {
#if DEBUG3
        fprintf(stderr, "leer_bit(): Error al llamar a bread()\n");
#endif
        return -1;
    }
    posbyte = posbyte % 8;
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha
    mascara &= buffMB[posbyte];  // operador AND para bits
    mascara >>= (7 - posbit);
    memset(buffMB, '\0', BLOCKSIZE);
    return mascara;
}

//Encuentra el primer bloque libre (consultando el MB), lo ocupa y devuelve su posición
int reservar_bloque()
{
    unsigned int posBloqueMB;
    unsigned char bufferMB[BLOCKSIZE]; //leemos del MB
    unsigned char bufferAUX[BLOCKSIZE];
    struct superbloque buff;
    if (bread(0, &buff) < 0)
    {
#if DEBUG3
        fprintf(stderr, "reservar_bloque(): Error al llamar a bread()\n");
#endif
        return -1;
    }
    posBloqueMB = buff.posPrimerBloqueMB; //leemos del primer bloque
    memset(bufferAUX, 255, BLOCKSIZE);    //buffer de 1s para comparar

    int bloque = 0;
    int byte = 0;
    unsigned int posbyte = 0;
    unsigned char mascara = 128; //10000000
    int posbit = 0;

    if (buff.cantBloquesLibres <= 0)
    {
        return -1;
    }

    //Buscamos el primer bloque con una posición libre
    while ((posBloqueMB < buff.posUltimoBloqueMB) && (bloque == 0))
    {
        if (bread(posBloqueMB, bufferMB) < 0)
        {
#if DEBUG3
            fprintf(stderr, "reservar_bloque(): Error al llamar a bread()\n");
#endif
            return -1;
        }
        if (memcmp(bufferMB, bufferAUX, BLOCKSIZE) < 0)
        {
            bloque = 1;
        }
        else
        {
            posBloqueMB++;
        }
    }
    //Encontramos la posición del primer byte libre
    while ((posbyte < BLOCKSIZE) && (byte == 0))
    {
        if (bufferMB[posbyte] != 255)
        {
            byte = 1;
        }
        else
        {
            posbyte++;
        }
    }

    // encontrar el primer bit a 0 en ese byte
    while (bufferMB[posbyte] & mascara)
    {                            // operador AND para bits
        bufferMB[posbyte] <<= 1; // desplazamiento de bits a la izquierda
        posbit++;
    }
    unsigned char auxZeroBuff[BLOCKSIZE];
    memset(auxZeroBuff, '\0', BLOCKSIZE); //buffer de 0s para eliminar basura
    unsigned int nbloque = ((posBloqueMB - buff.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    if (escribir_bit(nbloque, 1) < 0)
    {
#if DEBUG3
        fprintf(stderr, "reservar_bloque(): Error al llamar a escribir_bit()\n");
#endif
        return -1;
    }
    if (bwrite(nbloque, auxZeroBuff) < 0)
    {
#if DEBUG3
        fprintf(stderr, "reservar_bloque(): Error al llamar a bwrite()\n");
#endif
        return -1;
    }
    memset(auxZeroBuff, '\0', BLOCKSIZE);
    buff.cantBloquesLibres--; //Modificamos el superbloque
    if (bwrite(0, &buff) < 0) //Actualizamos el superbloque
    {
#if DEBUG3
        fprintf(stderr, "reservar_bloque(): Error al llamar a bwrite()\n");
#endif
        return -1;
    }
    return nbloque;
}

//Libera un bloque pasando su número de bloque por parámetro
//Retorna el número de bloque liberado o -1 en caso de error
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque aux;
    if (escribir_bit(nbloque, 0) < 0) //Modificamos el MB
    {
#if DEBUG3
        fprintf(stderr, "liberar_bloque(): Error al llamar a escribir_bit()\n");
#endif
        return -1;
    }
    if (bread(0, &aux) < 0)    //leer sb
    {
#if DEBUG3
        fprintf(stderr, "liberar_bloque(): Error al llamar a bread()\n");
#endif
        return -1;
    }
    //Modificamos la cantidad de bloques libres
    aux.cantBloquesLibres = aux.cantBloquesLibres + 1;

    if (bwrite(0, &aux) > 0) //Salvamos el SB modificado
    {
        return nbloque;
    }
    else
    {
#if DEBUG3
        fprintf(stderr, "librerar_bloque(): Error al llamar a bwrite()\n");
#endif
        return -1;
    }
}

int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    struct superbloque aux;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int posBloque;          //Posición del bloque donde queremos escribr
    if (bread(0, &aux) < 0) //Leemos el superbloque
    {
#if DEBUG3
        fprintf(stderr, "escribir_inodo(): Error al llamar a bread()\n");
#endif
        return -1;
    }
    posBloque = (ninodo / (BLOCKSIZE / INODOSIZE)) + aux.posPrimerBloqueAI;

    if (bread(posBloque, inodos) > 0) //Leemos el bloque donde se encuentra el inodo
    {
        //memcpy(&inodos[ninodo % (BLOCKSIZE / INODOSIZE)], &inodo, INODOSIZE);
        inodos[ninodo % (BLOCKSIZE / INODOSIZE)] = inodo;
        if (bwrite(posBloque, inodos) > 0)
        {
            return 0;
        }
        else
        {
#if DEBUG3
            fprintf(stderr, "escribir_inodo(): Error al llamar a bwrite() \n");
#endif
            return -1;
        }
    }
    else
    {
#if DEBUG3
        fprintf(stderr, "escribir_inodo(): Error al llamar a la funcion bread()\n");
#endif
        return -1;
    }
}

int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    //Se ha cambiado el array de inodos esta como puntero *revisar*
    struct superbloque aux;
    if (bread(0, &aux) < 0)
    {
#if DEBUG3
        fprintf(stderr, "leer_inodo(): Error funcion bread()\n");
#endif
        return -1;
    }
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int posBloque; //Posición del bloque donde queremos escribr

    posBloque = (ninodo / (BLOCKSIZE / INODOSIZE)) + aux.posPrimerBloqueAI;

    //Leemos el bloque donde se encuentra el inodo
    if (bread(posBloque, inodos) > 0)
    {
        *inodo = inodos[ninodo % (BLOCKSIZE / INODOSIZE)];
        return 0;
    }
    else
    {
#if DEBUG3
        fprintf(stderr, "leer_inodo(): Error funcion bread()\n");
#endif
        return -1;
    }
}

int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    unsigned int posInodoReservado;
    struct inodo auxInodo;
    struct superbloque aux;
    if (bread(0, &aux) < 0)
    {
#if DEBUG3
        fprintf(stderr, "reservar_inodo(): Error funcion bread()\n");
#endif
        return -1;
    }
    if (aux.cantInodosLibres != 0)
    {
        //Guardamos la posicion del primer inodo libre
        posInodoReservado = aux.posPrimerInodoLibre;
        if (leer_inodo(posInodoReservado, &auxInodo) < 0)
        {
#if DEBUG3
            fprintf(stderr, "reservar_inodo(): Error al leer inodo\n");
#endif
            return -1;
        }

        aux.posPrimerInodoLibre = auxInodo.punterosDirectos[0];
        aux.cantInodosLibres = aux.cantInodosLibres - 1;
        auxInodo.tipo = tipo;
        auxInodo.permisos = permisos;
        auxInodo.nlinks = 1;
        auxInodo.tamEnBytesLog = 0;
        auxInodo.atime = time(NULL);
        auxInodo.ctime = time(NULL);
        auxInodo.mtime = time(NULL);
        auxInodo.numBloquesOcupados = 0;

        memset(auxInodo.punterosDirectos, 0, sizeof(auxInodo.punterosDirectos));
        memset(auxInodo.punterosIndirectos, 0, sizeof(auxInodo.punterosIndirectos));
        //escribimos a inodo
        if (escribir_inodo(posInodoReservado, auxInodo) == -1)
        {
#if DEBUG3
            fprintf(stderr, "reservar_inodo(): error funcion escribir_inodo()\n");
#endif
            return -1;
        }
        if (bwrite(0, &aux) < 0) //Actualizamos el SB
        {
#if DEBUG3
            fprintf(stderr, "reservar_inodo(): Error al llamar a bwrite()\n");
#endif
            return -1;
        }
    }
    else
    {
#if DEBUG3
        fprintf(stderr, "reservar_inodo(): error cantidad de inodoslibres == 0 ()\n");
#endif
        return -1;
    }
    return posInodoReservado;
}

int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
#if DEBUG4
        fprintf(stderr, "obtener_nRangoBL(): Bloque lógico fuera de rango\n");
#endif
        return -1;
    }
}

int obtener_indice(int nblogico, int nivel_punteros)
{

    if (nblogico < DIRECTOS)
    {
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0)
    {
        return nblogico - DIRECTOS;
    }
    else if (nblogico < INDIRECTOS1)
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2)
    {
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return 0;
}

int traducir_bloque_inodo(int ninodo, int nblogico, char reservar)
{
    //Variables
    struct inodo inodo;
    unsigned int ptr, ptr_ant, salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];

    if (leer_inodo(ninodo, &inodo) < 0)
    {
#if DEBUG6
        fprintf(stderr, "traducir_bloque_inodo(): Error al leer inodo\n");
#endif
            return -1;
    }
    ptr = 0;
    ptr_ant = 0;
    salvar_inodo = 0;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr);
    if (nRangoBL < 0)
    {
#if DEBUG6
        fprintf(stderr, "traducir_bloque_inodo(): Error al llamar a obtener_nRangoBL()\n");
#endif
        return -1;
    }
    nivel_punteros = nRangoBL;
    while (nivel_punteros > 0) //Iterar para cada nivel de indirectos
    {
        if (ptr == 0)
        { //No cuelgan bloques de punteros
            if (reservar == 0)
            {
#if DEBUG6
                fprintf(stderr, "traducir_bloque_inodo(): No se ha reservado correctamente (reservar = 0)\n");
#endif
                return -1;
            }
            else
            {
                salvar_inodo = 1;
                ptr = reservar_bloque();
                if (ptr < 0)
                {
#if DEBUG6
                    fprintf(stderr, "traducir_bloque_inodo(): Error al reservar bloque\n");
#endif
                    return -1;
                }
                inodo.numBloquesOcupados = inodo.numBloquesOcupados + 1;
#if DEBUG6
                fprintf(stderr, "traducir_bloque_inodo(): inodo.numBloquesOcupados -> %d \n", inodo.numBloquesOcupados);
#endif
                inodo.ctime = time(NULL); //fecha actual
                if (nivel_punteros == nRangoBL)
                {
                    //El bloque cuelga directamente del inodo
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUG6
                    fprintf(stderr,"traducir_bloque_inodo(): inodo.punterosIndirectos[%d] = %d \n", nRangoBL - 1, ptr);
#endif
                }
                else
                {
                    buffer[indice] = ptr;
#if DEBUG6
                    fprintf(stderr,"traducir_bloque_inodo(): Valor puntero -> %d \n", ptr);
#endif
                    if (bwrite(ptr_ant, buffer) < 0)
                    {
#if DEBUG6
                        fprintf(stderr, "traducir_bloque_inodo(): Error al llamar a bwrite()\n");
#endif
                            return -1;
                    }
                }
            }
        }
        if (bread(ptr, buffer) < 0)
        {
#if DEBUG6
            fprintf(stderr, "traducir_bloque_inodo(): Error al llamar a bread()\n");
#endif
                return -1;
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        //guardamos el puntero
        ptr = buffer[indice]; //lo desplazamos al siguiente nivel
        nivel_punteros--;
    }
    //Ya estamos en el nivel de los datos
    if (ptr == 0)//No existe el bloque de datos
    { 
        if (reservar == 0)
        {
#if DEBUG6
            fprintf(stderr, "traducir_bloque_inodo(): No se ha reservado correctamente (reservar = 0)\n");
#endif
            return -1;
        }
        else
        {
            salvar_inodo = 1;
            ptr = reservar_bloque();
            if (ptr < 0)
            {
#if DEBUG6
                fprintf(stderr, "traducir_bloque_inodo(): Error al reservar bloque\n");
#endif
                return -1;
            }
#if DEBUG6
            fprintf(stderr,"traducir_bloque_inodo():  Valor puntero -> %d \n", ptr);
#endif
            inodo.numBloquesOcupados = inodo.numBloquesOcupados + 1;
#if DEBUG6
            fprintf(stderr,"traducir_bloque_inodo(): inodo.numBloquesOcupados -> %d \n", inodo.numBloquesOcupados);
#endif
            inodo.ctime = time(NULL);
            if (nRangoBL == 0)
            {
                inodo.punterosDirectos[nblogico] = ptr;
#if DEBUG6
                fprintf(stderr,"traducir_bloque_inodo(): indo.punterosDirectos[%d] = %d \n", nblogico, ptr);
#endif
            }
            else
            {
                buffer[indice] = ptr;
#if DEBUG6
                fprintf(stderr,"traducir_bloque_inodo(): Valor puntero -> %d \n", ptr);
#endif
                if (bwrite(ptr_ant, buffer) < 0)
                {
#if DEBUG6
                    fprintf(stderr, "traducir_bloque_inodo(): Error al llamar a bwrite() \n");
#endif
                    return -1;
                }
            }
        }
    }
    if (salvar_inodo == 1)
    {
        if (escribir_inodo(ninodo, inodo) < 0) //solo si lo hemos actualizado
        {
#if DEBUG4
            fprintf(stderr, "traducir_bloque_inodo(): Error al escribir inodo\n");
#endif
            return -1;
        }
    }
    return ptr; //nbfisico del bloque de datos
}

int liberar_inodo(unsigned int ninodo)
{
    struct inodo inodo; //leer el inodo
    if (leer_inodo(ninodo, &inodo) < 0)
    {
#if DEBUG6
        fprintf(stderr, "liberar_inodo(): Error al llamar a leer_inodo()\n");
#endif
        return -1;
    }
    struct superbloque SB;
    int blibres = liberar_bloques_inodo(0, &inodo); //vale 0 porque si liberamos un inodo hemos de liberar todos los otros
    if (blibres < 0)
    {
#if DEBUG6
        fprintf(stderr, "liberar_inodo(): Error al llamar a liberar_bloques_inodo()\n");
#endif
        return -1;
    }
#if DEBUG6
    fprintf(stderr,"liberar_inodo(): Numero de bloques liberados -> %d\n", blibres);
#endif

    if ((inodo.numBloquesOcupados - blibres) == 0)
    {
        inodo.tipo = 'l';
        inodo.tamEnBytesLog = 0;
        inodo.numBloquesOcupados = 0;
    }
    else
    {
#if DEBUG6
        fprintf(stderr, "liberar_inodo(): Error -> inodo.numBloquesOcupados - blibres != 0\n");
#endif
        return -1;
    }
    if (bread(0, &SB) < 0)
    {
#if DEBUG6
        fprintf(stderr, "liberar_inodo(): Error al llamar a bread()\n");
#endif
        return -1;
    }
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres = SB.cantInodosLibres + 1;
    if (bwrite(0, &SB) < 0)
    {
#if DEBUG6
        fprintf(stderr, "liberar_inodo(): Error al llamar a bwrite()\n");
#endif
        return -1;
    }
    if (escribir_inodo(ninodo, inodo) < 0)
    {
#if DEBUG6
        fprintf(stderr, "liberar_inodo(): Error al llamar a escribir_inodo()\n");
#endif
        return -1;
    }
    return ninodo;
}

// Libera los bloques de datos e índices iterando desde el primer bloque lógico a liberar hasta el último
// por tanto explora las ramificaciones de punteros desde las hojas hacia las raíces en el inodo
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    unsigned int ultimoBL, nivel_punteros, indice, ptr, nBL; //variables
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS]; //array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE];
    int ptr_nivel[3];  //punteros a bloques de punteros de cada nivel
    int indices[3];    //indices de cada nivel
    int liberados = 0; //nº de bloques liberados
    if (inodo->tamEnBytesLog == 0)
    {
        return 0; //el fichero esta vacío
    }
    if ((inodo->tamEnBytesLog % BLOCKSIZE) == 0)
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }
#if DEBUG6
    fprintf(stderr,"[liberar_bloques_inodo() --> primer BL: %d, último BL: %d]", primerBL, ultimoBL);
#endif
    memset(bufAux_punteros, '\0', BLOCKSIZE);
    ptr = 0;
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL < 0)
        {
#if DEBUG6
            fprintf(stderr, "liberar_bloques_inodo(): Error al llamar a obtener_nRangoBL()\n");
#endif
            return -1;
        }
        nivel_punteros = nRangoBL;
        while (ptr > 0 && nivel_punteros > 0)
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if ((indice == 0) || (nBL == primerBL)) //solo leemos del dispositivo si no está ya cargado previamente en un buffer
            {
                if (bread(ptr, bloques_punteros[nivel_punteros - 1]) < 0)
                {
#if DEBUG6
                    fprintf(stderr, "liberar_bloques_inodo(): Error al llamar a bread()\n");
#endif
                    return -1;
                }
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }
        if (ptr > 0)
        {
            if (liberar_bloque(ptr) < 0)
            {
#if DEBUG6
                fprintf(stderr, "liberar_bloques_inodo(): Error al llamar a liberar_bloque()\n");
#endif
                return -1;
            }
            liberados++;
#if DEBUG6
            fprintf(stderr, "[liberar_bloques_inodo() --> liberado BF %d de datos para BL %d] \n", ptr, nBL);
#endif
            if (nRangoBL == 0)
            {
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) //Posible error parentesis
                    {
                        if (liberar_bloque(ptr) < 0)
                        {
#if DEBUG6
                            fprintf(stderr, "liberar_bloques_inodo(): Error al llamar a liberar_bloque()\n");
#endif
                            return -1;
                        }
                        liberados++;
#if DEBUG6
                        fprintf(stderr, "[liberar_bloques_inodo() --> liberado BF %d de punteros_nivel%d correspondiente al BL %d\n]", ptr, nivel_punteros, nBL);
#endif
                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    }
                    else
                    {
                        if (bwrite(ptr, bloques_punteros[nivel_punteros - 1]) < 0)
                        {
#if DEBUG6
                            fprintf(stderr, "liberar_bloque_inodo(): Error al llamar a bwrite() \n");
#endif
                            return -1;
                        }
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
    }
#if DEBUG6
    fprintf(stderr, "\n[liberar_bloques_inodo() --> total bloques liberados %d\n", liberados);
#endif
    return liberados;
}
