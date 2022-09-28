//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "ficheros.h"
//Escribe el contenido procedente de un buffer de memoria, buf_original, de tamaño nbytes, en un
//fichero/directorio (correspondiente al inodo pasado como argumento, ninodo): le indicamos la posición
//de escritura inicial en bytes lógicos, offset, con respecto al inodo, y el número de bytes, nbytes,
//que hay que escribir. Hay que devolver la cantidad de bytes escritos realmente

int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    mi_waitSem();
    int escritos = 0;
    struct inodo inodoaux;
    if (leer_inodo(ninodo, &inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_write_f(): Error al leer inodo\n");
#endif  
        mi_signalSem();
        return -1;
    }
    if ((inodoaux.permisos & 2) != 2) //se mira si se tienen permisos de escritura
    {
#if DEBUG5
        fprintf(stderr, "mi_write_f():El inodo no tiene permisos de escritura\n");
#endif  
        mi_signalSem();
        return -1;
    }
    else
    {
        unsigned int primerBL = offset / BLOCKSIZE;                         //primer bloque donde hay que escribir
        unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE;          //ultimo bloque donde hay que escribir
        unsigned int desp1 = offset % BLOCKSIZE;                            //primer despalazamiento
        unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;             //segundo desplazamiento en el bloque para ver donde llegan los nbytes escritos a partir de offset
        unsigned char buf_bloque[BLOCKSIZE];                                //array de unsigned chars porque trabajamos con bytes
        unsigned int nbfisico = traducir_bloque_inodo(ninodo, primerBL, 1); //contenemos lo que nos devuelve traducir_bloque_inodo
        if (nbfisico < 0)
        {
#if DEBUG5
            fprintf(stderr, "mi_write_f(): Error al llamar a traducir_bloque_inodo()\n");
#endif      
            mi_signalSem();
            return -1;
        }
        if (primerBL == ultimoBL) //caso en que el primer y último bloque coincidan
        {
            if (bread(nbfisico, buf_bloque) < 0)
            { //leemos del bloque
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error bread nbfisico\n");
#endif          
                mi_signalSem();
                return -1;
            }
            memcpy(buf_bloque + desp1, buf_original, nbytes); //añadimos
            if (bwrite(nbfisico, buf_bloque) < 0)
            { //reescribimos
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error bwrite nbfisico\n");
#endif      
                mi_signalSem();
                return -1;
            }
            escritos = nbytes;
        }
        else
        { //caso en que la operación de escritura afecte a más de un bloque. Hay 3 partes:
            //la parte del primer bloque lógico
            if (bread(nbfisico, buf_bloque) < 0) //leemos del bloque
            {
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error bread nbfisico\n");
#endif          
                mi_signalSem();
                return -1;
            }
            memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1); //añadimos
            if (bwrite(nbfisico, buf_bloque) < 0)
            { //reescribimos
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error bwrite nbfisico\n");
#endif
                mi_signalSem();
                return -1;
            }
            escritos += BLOCKSIZE - desp1;
            //la parte de los bloques lógicos intermedios
            for (int i = primerBL + 1; i < ultimoBL; i++)
            { //cuantas veces itera, como lo sabemos;;
                nbfisico = traducir_bloque_inodo(ninodo, i, 1);
                if (nbfisico < 0)
                {
#if DEBUG5
                    fprintf(stderr, "mi_write_f(): Error al llamar a traducir_bloque_inodo()\n");
#endif              
                    mi_signalSem();
                    return -1;
                }
                if (bwrite(nbfisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE) < 0)
                { //reescribimos
#if DEBUG5
                    fprintf(stderr, "mi_write_f(): Error bwrite nbfisico\n");
#endif              
                    mi_signalSem();
                    return -1;
                }
                //esta situado aqui porque ya hemos leido de traducir_bloque_inodo en las declaraciones de arriba.
                escritos += BLOCKSIZE;
            }
            //la parte del ultimo bloque lógico
            nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 1);
            if (nbfisico < 0)
            {
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error al llamar a traducir_bloque_inodo()\n");
#endif          
                mi_signalSem();
                return -1;
            }
            if (bread(nbfisico, buf_bloque) < 0) //leemos del bloque
            {
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error bread nbfisico\n");
#endif          
                mi_signalSem();
                return -1;
            }
            memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
            if (bwrite(nbfisico, buf_bloque) < 0)
            { //reescribimos
#if DEBUG5
                fprintf(stderr, "mi_write_f(): Error bwrite nbfisico\n");
#endif  
                mi_signalSem();
                return -1;
            }
            escritos += (desp2 + 1);
        }
    }
    if (leer_inodo(ninodo, &inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_write_f(): Error al leer inodo\n");
#endif
        mi_signalSem();
        return -1;
    }
    //Finalmente actualizaremos la metainformación del inodo
    if (inodoaux.tamEnBytesLog < (offset + nbytes))
    {
        inodoaux.tamEnBytesLog = offset + nbytes;
        inodoaux.ctime = time(NULL);
    }
    inodoaux.mtime = time(NULL);

    if (escribir_inodo(ninodo, inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_write_f(): Error escribir_inodo\n");
#endif
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return escritos;
}

int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    mi_waitSem();
    int leidos = 0;
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_read_f(): Error al leer inodo\n");
#endif
        mi_signalSem();
        return -1;
    }
    if ((inodo.permisos & 4) != 4)
    {
#if DEBUG5
        fprintf(stderr, "mi_read_f(): El inodo no tiene permisos de lectura\n");
#endif
        mi_signalSem();
        return -1;
    }
    else
    {
        if (offset >= inodo.tamEnBytesLog)
        {
            leidos = 0;
            mi_signalSem();
            return leidos;
        }
        if ((offset + nbytes) >= inodo.tamEnBytesLog)
        {
            nbytes = inodo.tamEnBytesLog - offset;
        }
        unsigned int primerBL = offset / BLOCKSIZE;                //primer bloque donde hay que leer
        unsigned int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; //ultimo bloque donde hay que leer
        unsigned int desp1 = offset % BLOCKSIZE;                   //primer despalazamiento
        unsigned int desp2 = (offset + nbytes - 1) % BLOCKSIZE;    //segundo desplazamiento en el bloque para ver donde llegan los nbytes leidos a partir de offset
        unsigned char buf_bloque[BLOCKSIZE];                       //array de unsigned chars porque trabajamos con bytes
        unsigned int nbfisico;                                     //contenemos lo que nos devuelve traducir_bloque_inodo
        if (primerBL == ultimoBL)                                  //caso en que el primer y último bloque coincidan
        {
            if ((nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0)) == -1)
            {
                leidos += nbytes;
            }
            else
            {
                if (bread(nbfisico, buf_bloque) < 0) //leemos del bloque
                {
#if DEBUG5
                    fprintf(stderr, "mi_read_f(): Error bread nbfisico\n");
#endif
                    mi_signalSem();
                    return -1;
                }
                memcpy(buf_original, buf_bloque + desp1, nbytes);
                leidos = nbytes;
            }
        }
        else
        {
            if ((nbfisico = traducir_bloque_inodo(ninodo, primerBL, 0)) == -1)
            {
                leidos += BLOCKSIZE - desp1;
            }
            else
            {
                //la parte del primer bloque lógico
                if (bread(nbfisico, buf_bloque) < 0) //leemos del bloque
                {
#if DEBUG5
                    fprintf(stderr, "mi_read_f(): Error bread nbfisico\n");
#endif
                    mi_signalSem();
                    return -1;
                }
                memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1); //añadimos
                leidos += (BLOCKSIZE - desp1);
            }
            //la parte de los bloques lógicos intermedios
            for (int i = primerBL + 1; i < ultimoBL; i++)
            {
                if ((nbfisico = traducir_bloque_inodo(ninodo, i, 0)) != -1)
                {
                    if (bread(nbfisico, buf_bloque) < 0) //leemos del bloque
                    {
#if DEBUG5
                        fprintf(stderr, "mi_read_f(): Error bread nbfisico\n");
#endif
                        mi_signalSem();
                        return -1;
                    }
                    memcpy(buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE, buf_bloque, BLOCKSIZE);
                    leidos = leidos + BLOCKSIZE;
                }
                else
                {
                    leidos = leidos + BLOCKSIZE;
                }
            }
            //la parte del ultimo bloque lógico
            if ((nbfisico = traducir_bloque_inodo(ninodo, ultimoBL, 0)) != -1)
            {
                if (bread(nbfisico, buf_bloque) < 0) //leemos del bloque
                {
#if DEBUG5
                    fprintf(stderr, "mi_read_f(): Error bread nbfisico\n");
#endif
                    mi_signalSem();
                    return -1;
                }
                memcpy(buf_original + (BLOCKSIZE - desp1) + (ultimoBL - primerBL - 1) * BLOCKSIZE, buf_bloque, desp2 + 1);
                leidos = leidos + (desp2 + 1);
            }
            else
            {
                leidos = leidos + (desp2 + 1);
            }
            if (leer_inodo(ninodo, &inodo) < 0)
            {
#if DEBUG5
                fprintf(stderr, "mi_read_f(): Error al leer inodo\n");
#endif  
                mi_signalSem();
                return -1;
            }
            inodo.atime = time(NULL);

            if (escribir_inodo(ninodo, inodo) < 0)
            {
#if DEBUG5
                fprintf(stderr, "mi_read_f(): Error al escribir inodo\n");
#endif
                mi_signalSem();
                return -1;
            }
        }
    }
    mi_signalSem();
    return leidos;
}

int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodoaux;
    if (leer_inodo(ninodo, &inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_stat_f(): Error al leer inodo\n");
#endif
        return -1;
    }
    p_stat->tipo = inodoaux.tipo;
    p_stat->permisos = inodoaux.permisos;
    p_stat->atime = inodoaux.atime;
    p_stat->mtime = inodoaux.mtime;
    p_stat->ctime = inodoaux.ctime;
    p_stat->nlinks = inodoaux.nlinks;
    p_stat->tamEnBytesLog = inodoaux.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodoaux.numBloquesOcupados;
    return 0;
}

int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    mi_waitSem();
    struct inodo inodoaux;
    if (leer_inodo(ninodo, &inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_chmod_f(): Error al leer inodo\n");
#endif
        mi_signalSem();
        return -1;
    }
    inodoaux.permisos = permisos;
    inodoaux.ctime = time(NULL);
    if (escribir_inodo(ninodo, inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_chmod_f(): Error escribir inodo\n");
#endif  
        mi_signalSem();
        return -1;
    }
    mi_signalSem();
    return 0;
}

int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodoaux;
    if (leer_inodo(ninodo, &inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_truncar_f(): Error al leer inodo\n");
#endif
        return -1;
    }
    if ((inodoaux.permisos & 2) != 2) //Se mira si se tienen permisos de escritura
    {
#if DEBUG6
        fprintf(stderr, "mi_truncar_f(): Permisos no validos %d\n", inodoaux.permisos);
#endif
        return -1;
    }
    //Se comprueba que se puede truncar el fichero
    if (nbytes > inodoaux.tamEnBytesLog)
    {
#if DEBUG6
        fprintf(stderr, "mi_truncar_f(): No se puede truncar un fichero más allá de su tamaño en bytes lógicos. \n");
#endif
        return -1;
    }
    int primerBL;
    if ((nbytes % BLOCKSIZE) == 0)
    {
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = (nbytes / BLOCKSIZE) + 1;
    }
    int blibres = liberar_bloques_inodo(primerBL, &inodoaux);
    if (blibres < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_truncar_f(): Error al llamar a liberar_bloques_inodo()\n");
#endif
        return -1;
    }
    inodoaux.numBloquesOcupados = inodoaux.numBloquesOcupados - blibres;
    inodoaux.mtime = time(NULL);
    inodoaux.ctime = time(NULL);
    inodoaux.tamEnBytesLog = nbytes;
    fprintf(stderr,"nbytes: %d\n",nbytes);
    if (escribir_inodo(ninodo, inodoaux) < 0)
    {
#if DEBUG5
        fprintf(stderr, "mi_truncar_f(): Error al escribir inodo\n");
#endif
        return -1;
    }
    return blibres;
}