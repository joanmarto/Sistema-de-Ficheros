//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "bloques.h"

static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;

void mi_waitSem()
{
    if (!inside_sc)
    { // inside_sc==0
        waitSem(mutex);
    }
    inside_sc++;
}
void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
}
//Abre el fichero indicado en el path y consigue el valor del descriptor o -1 en caso de error.
int bmount(const char *camino)
{
    umask(000);
    if (descriptor > 0)
    {
        close(descriptor);
    }
    if ((descriptor = open(camino, O_RDWR | O_CREAT, 0666)) == -1)
    {
#if DEBUG1
        fprintf(stderr, "bmount() --> Error descriptor = -1\n");
#endif
    }
    if (!mutex)
    { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return -1;
        }
    }
    return descriptor;
}

//Devuelve 0  si se ha cerrado el fichero correctamente, o -1 en caso contrario
int bumount()
{
    descriptor = close(descriptor);
    if (descriptor == -1)
    {
#if DEBUG1
        fprintf(stderr, "bumount() --> error aux = -1\n");
#endif
        return -1;
    }
    deleteSem();
    return 0;
}

//Escribe de un bloque en el dispositivo virtual, en el bloque físico especificado por nbloque
int bwrite(unsigned int nbloque, const void *buf)
{
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    size_t aux = write(descriptor, buf, BLOCKSIZE);
    if (aux == BLOCKSIZE)
    {
        return aux;
    }
    else
    {
        return -1;
    }
}

//Lectura de un bloque. Retorna el tamaño del bloque leido o -1 en caso de error
int bread(unsigned int nbloque, void *buf)
{
    lseek(descriptor, nbloque * BLOCKSIZE, SEEK_SET);
    size_t aux = read(descriptor, buf, BLOCKSIZE);
    if (aux < 0)
    {
#if DEBUG1
        fprintf(stderr, "bread() --> Error read() = -1\n");
#endif
        return -1;
    }
    return aux;
}