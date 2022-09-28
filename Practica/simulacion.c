//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "simulacion.h"
static int acabados;

int main(int argc, char **argv)
{
    signal(SIGCHLD, reaper); //Asignamos la señal SIGCHLD al enterrador
    pid_t pid;
    if (argc != 2)
    {
        fprintf(stderr, "Sintaxis no válida: ./simulacion <disco> \n");
        return -1;
    }
    if (bmount(argv[1]) < 0)
    {
#if DEBUG12
        fprintf(stderr, "simulacion.c -> Error al montar el dispositivo\n");
#endif
        return -1;
    }
    char tmp[20]; //Cosas para sacar el nombre
    char dir[100];
    time_t t = time(NULL);
    memset(dir, '\0', sizeof(dir));
    memset(tmp, '\0', sizeof(tmp));

    tm = localtime(&t);
    sprintf(tmp, "%d%02d%02d%02d%02d%02d/", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
    strcpy(dir, "/simul_");
    strcat(dir, tmp);
    if ((mi_creat(dir, 6)) < 0)
    { //simul_aaaammddhhmmss/ y permisos
#if DEBUG12
        fprintf(stderr, "simulacion.c --> Error: creando el directorio %s\n", dir);
#endif
        if (bumount() < 0)
        {
#if DEBUG12
            fprintf(stderr, "simulacion.c --> Error: bumount()\n");
#endif
            return -1;
        }
        return -1;
    }

    for (int proceso = 1; proceso <= NUMPROCESOS; proceso++)
    {
        pid = fork();
        if (pid == 0) //Es el hijo
        {
            if (bmount(argv[1]) < 0)
            {
#if DEBUG12
                fprintf(stderr, "simulacion.c --> Error: Al montar el disco del proceso %d\n", getpid());
#endif
                return -1;
            }
            char diraux[100];
            char dirpid[50];
            memset(diraux, '\0', sizeof(diraux));
            memset(dirpid, '\0', sizeof(dirpid));
            strcpy(diraux, dir);
            sprintf(dirpid, "proceso_%d", getpid());
            strcat(dirpid, "/");
            strcat(diraux, dirpid);
            if ((mi_creat(diraux, 6)) < 0) // /simul_aaaammddhhmmss/proceso_PIDX/ y permisos
            {
#if DEBUG12
                fprintf(stderr, "simulacion.c --> Error del proceso %d: creando el directorio %s\n", getpid(), diraux);
#endif
                if (bumount() < 0) //Padre
                {
#if DEBUG12
                    fprintf(stderr, "simulacion.c --> Error: bumount()\n");
#endif
                    return -1;
                }
                return -1;
            }
#if DEBUG12
            fprintf(stderr, "\nCreado con exito el directorio %s\n", diraux);
#endif
            strcat(diraux, "prueba.dat"); //Crear fichero.dat
#if DEBUG12
            fprintf(stderr, "\n%s\n", diraux);
#endif
            if ((mi_creat(diraux, 6)) < 0)
            { // simul_aaaammddhhmmss/proceso_PIDX/prueba.dat y permisos
#if DEBUG12
                fprintf(stderr, "simulacion.c --> Error del proceso %d creando el directorio %s\n", getpid(), diraux);
#endif
                if (bumount() < 0)
                {
#if DEBUG12
                    fprintf(stderr, "simulacion.c --> Error: bumount()\n");
#endif
                    return -1;
                }
                return -1;
            }
            srand(time(NULL) + getpid());
            int nescritura;
            for (nescritura = 1; nescritura <= NUMESCRITURAS; nescritura++) //Inicializamos el registro
            {
                struct REGISTRO rg;
                rg.fecha = time(NULL);
                rg.pid = getpid();
                rg.nEscritura = nescritura;
                rg.nRegistro = rand() % REGMAX;
                //Escribimos el registro
                //fprintf(stderr, "[simulación.c → Escritura %d en %s]\n",proceso,diraux);
                if (mi_write(diraux, &rg, rg.nRegistro * sizeof(struct REGISTRO), sizeof(struct REGISTRO)) < 0)
                {
#if DEBUG12
                    fprintf(stderr, "simulacion.c mi_write() --> Error del proceso %d en la escritura\n", getpid());
#endif
                    exit(1);
                }
                usleep(50000); //Esperamos 0,05 segundos
            }

            fprintf(stderr, "[Proceso %d: Completadas %d escrituras en %s]\n", proceso, nescritura, diraux);
            if (bumount() < 0)
            {
#if DEBUG12
                fprintf(stderr, "simulacion.c --> Error: bumount()\n");
#endif
                return -1;
            }
            exit(0); //Necesario para que se emita la señal SIGCHLD
        }
        usleep(200000); //Esperamos 0,2 segundos
    }
    //Permitir que el padre espere por todos los hijos
    while (acabados < NUMPROCESOS)
    {
        pause();
    }
    if (bumount() < 0) //Padre
    {
#if DEBUG12
        fprintf(stderr, "simulacion.c --> Error: bumount()\n");
#endif
        return -1;
    }
    exit(0);
}

void reaper() //Función enterrador
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}