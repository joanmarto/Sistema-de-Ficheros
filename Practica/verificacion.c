//Pau Bonet Alcover, Joan Martorell Ferriol, Zhuo Han Yang
#include "verificacion.h"

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "Uso: ./verificacion <nombre_dispositivo> <directorio_simulación>");
    exit(-1);
  }
  char path[100];
  if (bmount(argv[1]) < 0)
  { //montamos el disco
#if DEBUG13
    fprintf(stderr, "verificar.c --> Error bmount()");
#endif
    return -1;
  }
  struct STAT stat;
  if ((mi_stat(argv[2], &stat)) < 0)
  {
#if DEBUG13
    fprintf(stderr, "verificar.c --> Error mi_stat()");
#endif
    return -1;
  }
  int numentradas = stat.tamEnBytesLog / sizeof(struct entrada); //numero de entradas que hay en el inodo
  if (numentradas != NUMPROCESOS)
  {
#if DEBUG13
    fprintf(stderr, "ERROR: numentradas != numescriutras");
#endif
    return -1;
  }

  char informe[100]; //camino del informe
  char auxinfo[15];
  memset(informe, '\0', sizeof(informe));
  memset(auxinfo, '\0', sizeof(auxinfo));
  strcpy(informe, argv[2]);
  sprintf(auxinfo, "informe.txt");
  strcat(informe, auxinfo);
#if DEBUG13
  fprintf(stderr, "verificación.c --> Camino informe: %s\n", informe);
#endif
  if ((mi_creat(informe, 7)) < 0)
  {
#if DEBUG13
    fprintf(stderr, "verificación.c --> Error al crear el fichero informe\n");
#endif
    return -1;
  }
  int offsetInfo = 0;
  struct entrada *ent;
  ent = malloc(sizeof(struct entrada));
  unsigned char buff_ent[NUMPROCESOS * sizeof(struct entrada)];
  char *strpid;
  memset(buff_ent, '\0', sizeof(buff_ent));
  if ((mi_read(argv[2], &buff_ent, 0, numentradas * sizeof(struct entrada))) < 0) //buffer con todas las entradas
  {
#if DEBUG13
    fprintf(stderr, "verificación.c --> Error: mi_read()\n");
#endif
    return -1;
  }
  struct INFORMACION informacion;
  for (int i = 0; i < numentradas; i++)
  {
    memset(path, '\0', sizeof(path));
    strcpy(path, argv[2]);

    char dirpid[50];
    memset(dirpid, '\0', sizeof(dirpid));
    memcpy(ent, buff_ent + (i * sizeof(struct entrada)), sizeof(struct entrada));

    strcpy(dirpid, ent->nombre);
    strpid = strchr(dirpid, '_'); //cogemos el PID del PROCESO
    int pid = atoi(strpid + 1);
    informacion.pid = pid;

    int offset = 0;
    strcat(path, ent->nombre);
    strcat(path, "/prueba.dat");

    char buff_write[256];
    char buff_write_aux[100];

    int cant_registros_buffer_escrituras = 256;
    struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
    int contador = 0;

    while (mi_read(path, &buffer_escrituras, offset, sizeof(buffer_escrituras)) > 0) //cuando mi_read no tenga nada mas que ller dara -1, y saldra del while
    {
      for (int i = 0; i < cant_registros_buffer_escrituras; i++)
      {
        if (buffer_escrituras[i].pid == pid) //escritura validada si entra
        {
          if (contador == 0)
          {
            informacion.PrimeraEscritura = buffer_escrituras[i];
            informacion.UltimaEscritura = buffer_escrituras[i];
            informacion.MenorPosicion = buffer_escrituras[i];
            informacion.MayorPosicion = buffer_escrituras[i];
            contador++;
          }
          else
          {
            if (informacion.PrimeraEscritura.nEscritura > buffer_escrituras[i].nEscritura)
            {
              informacion.PrimeraEscritura = buffer_escrituras[i];
            }
            if (informacion.UltimaEscritura.nEscritura < buffer_escrituras[i].nEscritura)
            {
              informacion.UltimaEscritura = buffer_escrituras[i];
            }
            if (informacion.MenorPosicion.nRegistro > buffer_escrituras[i].nRegistro)
            {
              informacion.MenorPosicion = buffer_escrituras[i];
            }
            if (informacion.MayorPosicion.nRegistro < buffer_escrituras[i].nRegistro)
            {
              informacion.MayorPosicion = buffer_escrituras[i];
            }
            contador++;
          }
          informacion.nEscrituras = contador; //obtener escrituras de la última posición
        }
      }
      offset += sizeof(buffer_escrituras);
      memset(buffer_escrituras, '\0', sizeof(buffer_escrituras)); //vaciamos el buffer cada vez
    }
    //Escribimos la información
    memset(buff_write, '\0', sizeof(buff_write));
    memset(buff_write_aux, '\0', sizeof(buff_write_aux));
    sprintf(buff_write, "\nPID: %u\n", pid);

    sprintf(buff_write_aux, "Numero de escrituras: \t%d\n", contador);
    strcat(buff_write, buff_write_aux);
    memset(buff_write_aux, '\0', sizeof(buff_write_aux));

    sprintf(buff_write_aux, "Primera escritura:\t%d\t%d\t%s\n", informacion.PrimeraEscritura.nEscritura, informacion.PrimeraEscritura.nRegistro, asctime(localtime(&informacion.PrimeraEscritura.fecha)));
    strcat(buff_write, buff_write_aux);
    memset(buff_write_aux, '\0', sizeof(buff_write_aux));

    sprintf(buff_write_aux, "Ultima escritura:\t%d\t%d\t%s\n", informacion.UltimaEscritura.nEscritura, informacion.UltimaEscritura.nRegistro, asctime(localtime(&informacion.UltimaEscritura.fecha)));
    strcat(buff_write, buff_write_aux);
    memset(buff_write_aux, '\0', sizeof(buff_write_aux));

    sprintf(buff_write_aux, "Menor posicion:\t\t%d\t%d\t%s\n", informacion.MenorPosicion.nEscritura, informacion.MenorPosicion.nRegistro, asctime(localtime(&informacion.MenorPosicion.fecha)));
    strcat(buff_write, buff_write_aux);
    memset(buff_write_aux, '\0', sizeof(buff_write_aux));

    sprintf(buff_write_aux, "Mayor posicion:\t\t%d\t%d\t%s\n", informacion.MayorPosicion.nEscritura, informacion.MayorPosicion.nRegistro, asctime(localtime(&informacion.MayorPosicion.fecha)));
    strcat(buff_write, buff_write_aux);
    memset(buff_write_aux, '\0', sizeof(buff_write_aux));
    fprintf(stderr, "informe: %s\n", informe);

    if ((mi_write(informe, buff_write, offsetInfo, sizeof(buff_write))) < 0)
    {
#if DEBUG13
      fprintf(stderr, "verificación.c --> Error: escribiendo el informe\n");
#endif
      return -1;
    }
    write(1, buff_write, strlen(buff_write));
#if DEBUG13
    fprintf(stderr, "%d\n", offsetInfo);
#endif
    offsetInfo += sizeof(buff_write);
  }
  if (bumount() < 0)
  {
#if DEBUG13
    fprintf(stderr, "verificacion.c --> Error: bumount()\n");
#endif
    return -1;
  }
  return 0;
}