#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/wait.h>

void imprimir();
void tuberia();

typedef struct Lista
{
	char cad[100];
	struct Lista *anterior;
}Lista;


int main(int argc, char* argv[]) {

	DIR *dp;
	struct dirent *ep;
	struct stat datosFichero;
	char *pCh;
	char ruta[100]; //ruta en la que introducimos la ruta del directorio
	char cadena[100]; //variable que se modificara por cada archivo a leer
	Lista *lineaActual = 0;

  int size = 0;

	if(argc<2 || !strcmp(argv[1],"-h"))
	{
		fprintf(stdout,
				"busca -h: Muestra la ayuda\n"
				"busca dir [-n|-t|-p|-c|-C] argumento [-R] "
				"[-print|-pipe|-exec] [mandato argumentos..]\n");
		return 0;
	}
	if(argc>4)
	{
   dp = opendir(argv[1]);
		if(!dp && (errno== 2 || errno==20 || errno == 13))
    {
			fprintf(stderr,"%s: Error(2), No se puede leer el directorio\n", argv[0]);
			exit(2);
		}
    //obligamos a que haya un 4 argumento con ese nombre
    if(!(!strcmp(argv[4],"-pipe")||(!strcmp(argv[4],"-print"))||(!strcmp(argv[4],"-exec"))))
       {
        fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
				exit(1);
       } 
   	strcpy(ruta,argv[1]); // para añadir la ruta
	  strcat(ruta,"/");
                                                                                  /* Criterio de seleccion : tipo de archivo */
		if(!strcmp(argv[2],"-t"))
    {
			while (ep = readdir(dp))
      {
				strcpy(cadena,ruta);
				strcat(cadena, ep->d_name);
				lstat(cadena,&datosFichero);
             
             //fichero                   
			       
						if(!strcmp(argv[3],"f"))
          {
           if(S_ISREG(datosFichero.st_mode))
					 {
          //   if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))   //para archivos ocultos
          //   {
             if(strcspn(ep->d_name,".")!=0)
             {
					  	 if(strcmp(argv[4],"-print")==0)
						   {
						  	 printf("%s/%s\n",argv[1],ep->d_name);
						   }
             
               
						else
						{
							Lista *nuevaLinea = (Lista*) malloc(sizeof(Lista));
							if(nuevaLinea != NULL) //queda memoria en el sistema
							{
								strcpy(nuevaLinea->cad, cadena);
                 if(lineaActual==NULL)
                 {    
                  lineaActual=nuevaLinea;    
                  }
								nuevaLinea->anterior = lineaActual;
								lineaActual = nuevaLinea;
								size++;
							}
						}
            }
            }
           }
				 
              //directorio
             
				else if(strcmp(argv[3],"d")==0)
        {
          if(ep->d_type & DT_DIR)
				  {		
          if(!strstr(ep->d_name,"."))      // no imprimimos los que empiecen por .  
	         {		
						if(!strcmp(argv[4],"-print"))
					 	 {
             printf("%s/%s\n",argv[1],ep->d_name);
						  }
						else
						{
							Lista *nuevaLinea = (Lista*) malloc(sizeof(Lista));
							if(nuevaLinea != NULL) 
							{
								strcpy(nuevaLinea->cad, cadena);
								nuevaLinea->anterior = lineaActual;
								lineaActual = nuevaLinea;
								size++;
							}
						}
            }
           }				
				}
				else
				{
					fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
					exit(1);
				}
			}   //end while
			if(!strcmp(argv[4],"-pipe"))
			{
				tuberia(size, lineaActual, argv);
			}
      
      
		} //end if selección por tipo de archivo
                                                                               /*Criterio de selección : por nombre*/
                                               
	  else if(!strcmp(argv[2],"-n"))
		{
			while (ep = readdir(dp))
			{			
				if(strstr(ep->d_name, argv[3]))
				{
					if(!strcmp(argv[4],"-print"))
					{
						 if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
              
					}
					else
					{
						Lista *nuevaLinea = (Lista*) malloc(sizeof(Lista));
						if(nuevaLinea != NULL) //queda memoria en el sistema
						{
							strcpy(nuevaLinea->cad, cadena);
							nuevaLinea->anterior = lineaActual;   //movemos el puntero
							lineaActual = nuevaLinea;
							size++;
						}
					}
				}
			}
			if(!strcmp(argv[4],"-pipe"))
			{
				tuberia(size, lineaActual, argv);
			}
		} // end if selección por nombre de archivo
                                                                       /*Criterio de selección : permisos de archivo*/
		 if(strcmp(argv[2],"-p") == 0)
		{
			while (ep = readdir(dp))
			{
				strcpy(cadena,ruta);
				strcat(cadena, ep->d_name);
				lstat(cadena,&datosFichero);
				int modo = 0;
				if(strcmp(argv[3],"r") == 0)
					modo=1;
				else if(strcmp(argv[3],"w") == 0)
					modo=2;
				else if(strcmp(argv[3],"x") == 0)
					modo=3;
				else
				{
					fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
					exit(1);
				}   
         //usuario
				if(datosFichero.st_uid == geteuid())
				{
					switch(modo)
					{
					case 1:
						if((datosFichero.st_mode & S_IRUSR)!=0)
						{
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
               if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					case 2:
						if((datosFichero.st_mode & S_IWUSR)!=0)
							{
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
               if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					case 3:
						if((datosFichero.st_mode & S_IXUSR)!=0)
								 {
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					}
				}
        //grupo
				else if(datosFichero.st_gid == getegid())
				{
					switch(modo)
					{
					case 1:
						if((datosFichero.st_mode & S_IRGRP)!=0)
							{
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					case 2:
						if((datosFichero.st_mode & S_IWGRP)!=0)
							{
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
					case 3:
						if((datosFichero.st_mode & S_IXGRP)!=0)
							 {
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                 if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					}
				}
				else {
        //otros
					switch(modo)
					{
					case 1:
						if((datosFichero.st_mode & S_IROTH)!=0)
           {
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                 if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					case 2:
						if((datosFichero.st_mode & S_IWOTH)!=0)
							 {
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                 if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					case 3:
						if((datosFichero.st_mode & S_IXOTH)!=0)
							{
              // acción que debemos aplicar: imrpimir l
            if(!strcmp(argv[4],"-print"))
            {
                 if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
              {       
             printf("%s/%s\n",argv[1],ep->d_name);
              }
              //En este punto podemos tener directorios que no contengan .
             else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
              {
                  printf("%s/%s\n",argv[1],ep->d_name);
              }
            }
            else if(!strcmp(argv[4],"-pipe"))
            {
               tuberia(size, lineaActual, argv);
            }
            else      //no implementado
             {
              fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
              exit(1);
             }
             } //end if mode
						break;
					}
				}
			}
		}
   
    
		if(strcmp(argv[2],"-c") == 0)
		{

			while (ep = readdir(dp)){
				strcpy(cadena,ruta); //Copiamos la ruta del directorio
				strcat(cadena, ep->d_name); //creamos dirección absoluta
				lstat(cadena,&datosFichero);
				if(S_ISREG(datosFichero.st_mode))
				{
					int fd;
					char buffer[datosFichero.st_size];
					fd = open(cadena,O_RDONLY);
					if (fd > 0)
					{
						read(fd, buffer, datosFichero.st_size);				
						if(strstr(buffer, argv[3])!=0)
						{
							if(!strcmp(argv[4],"-print"))
							{
						    if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
                {       
                printf("%s/%s\n",argv[1],ep->d_name);
                }
                //En este punto podemos tener directorios que no contengan .
                else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
                {
                  printf("%s/%s\n",argv[1],ep->d_name);
                }
							}
							else
							{
								Lista *nuevaLinea = (Lista*) malloc(sizeof(Lista));
								if(nuevaLinea != NULL) //queda memoria en el sistema
								{
									strcpy(nuevaLinea->cad, cadena);
									nuevaLinea->anterior = lineaActual;
									lineaActual = nuevaLinea;
									size++;

								}
							}
						}
					}
					else
					{
						fprintf(stderr,"%s: Error(3), No se puede leer el archivo\n", argv[0]);
					}
					close(fd);
				}
			}
			if(!strcmp(argv[4],"-pipe"))
			{
				tuberia(size, lineaActual, argv);
			}
		}
		if(strcmp(argv[2],"-C") == 0)
		{
			while (ep = readdir(dp)){
				strcpy(cadena,ruta); //Copiamos la ruta del directorio
				strcat(cadena, ep->d_name); //A–adimos el nombre del archivo para crear su ruta completa
				lstat(cadena,&datosFichero);
				if(S_ISREG(datosFichero.st_mode))
				{

					int fd;
					fd = open(cadena,O_RDONLY);
					if(fd > 0)
					{
						char *memoria;
						memoria = mmap(0,datosFichero.st_size,PROT_READ,MAP_SHARED,fd,0);//devuelve la direccion de memoria donde se ubica o MAP_FAILED si no se puede ubicar.
						if(memoria<0)
						{
							fprintf(stderr,"%s: Error(?), No se puede mapear\n", argv[0]);

						}
						else
						{	
							if(strstr(memoria, argv[3])!=0)
							{
								if(!strcmp(argv[4],"-print"))
								{
							    if((ep->d_type & DT_REG)||(strstr(ep->d_name,".")!=0)&!(ep->d_type & DT_DIR))
                  {       
                  printf("%s/%s\n",argv[1],ep->d_name);
                  }
                  //En este punto podemos tener directorios que no contengan .
                  else if((strstr(ep->d_name,".")==NULL))  //para imprimir los directorios que no contengan .
                   {
                    printf("%s/%s\n",argv[1],ep->d_name);
                  }
								}
								else
								{
									Lista *nuevaLinea = (Lista*) malloc(sizeof(Lista));
									if(nuevaLinea != NULL) //queda memoria en el sistema
									{
										strcpy(nuevaLinea->cad, cadena);
										nuevaLinea->anterior = lineaActual;
										lineaActual = nuevaLinea;
										size++;

									}
								}
							}
						}
						munmap(memoria,datosFichero.st_size);//desmapeamos memoria 
					}
					else
					{
						fprintf(stderr,"%s: Error(3), No se puede leer el archivo\n", argv[0]);
					}

					close(fd);
				}
			}
			if(!strcmp(argv[4],"-pipe"))
			{
				tuberia(size, lineaActual, argv);
			}
		}
		return 0;
	}
  else
  {
        fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
				exit(1);
  }
}

/***************Funcion Accion a Aplicar ***************/

void tuberia(int size, Lista* lineaActual, char *argv[]){
	if(argv[5]!=NULL)
	{
		int fd[2];
		int pid,pid2,pid3;
    int ret;
    int status;
    signal(SIGPIPE,SIG_IGN);
		if (pipe(fd)<0){
			perror("pipe");
			exit(1);
		}
		pid = fork();
		switch(pid)
		{
		case -1:
		{
			close(fd[0]);
			close(fd[1]);
			perror("fork");
			exit(1);
		}
    case 0: //hijo crea un nuevo proceso.
		 {
         close(fd[0]);   
         close(1);
         dup(fd[1]);     //duplicamos el descriptor de escritura.
         close(fd[1]);   //cerramos el descriptor de escritura.
         pid2=fork();
         switch(pid2)
         {
          case -1:
            perror("fork");
            exit(1);
          case 0:
             while(size>=0)
		  	   {
		  		 write(fd[1], lineaActual->cad, strlen(lineaActual->cad));
            lineaActual = lineaActual->anterior;
          // if(size!=0)
          //   write(fd[1],"\n",4);
	            size--;
            }
          default:
            wait(NULL);
            exit(0);
         }  
         default:
           close(fd[1]);
           close(0);
           dup(fd[0]);
           close(fd[0]); 
           pid3= fork();
           switch(pid3)
           {
            case -1:
            perror("fork");
            exit(1);
            case 0:
            execvp(argv[5],argv+5);
            fprintf(stderr,"%s: Error(4), No se puede ejecutar el mandato\n", argv[0]);
            exit(1);
            default:
            wait(NULL);
            wait(NULL);
            exit(0);
           }                                                                                                     	       
		     }
      }
    }
    else
    {
     fprintf(stderr,"%s: Error(1), Opcion no valida\n", argv[0]);
     exit(1);
    }
	}