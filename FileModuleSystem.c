#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdbool.h>

int list(bool , char*  , char* ,bool);
int parse(char*);
int extract (char *, int , int);
int findall(char*);

int main(int argc, char **argv) {
    if (argc >= 2) {
        if(strcmp(argv[1], "variant") == 0)
        // s-a ales metoda variant 
            printf("62428\n");
        else if(strcmp(argv[1],"list")==0) {
        // s-a ales metoda list si se declara cate o variabila pt toate filtrele posibile, dupa care se verifica toate filtrele aplicate si se modifica 
        // variabilele declarate
            int nr_arg=2;
            bool rec=false;
            bool perm=false;
            char *end = NULL;
            char *path = NULL;
            //se ia fiecare argument pe rand si se verifica ce filtru/optiune reprezinta
            while (nr_arg<argc) {
                if (strcmp(argv[nr_arg],"recursive")==0) 
                    rec=true;
                else if (strncmp(argv[nr_arg],"path=",5)==0)
                    path=argv[nr_arg]+5;
                else if (strncmp(argv[nr_arg],"name_ends_with=",15)==0) 
                    end=argv[nr_arg]+15;
                else if (strncmp(argv[nr_arg],"has_perm_write",14)==0)
                    perm=true;
                nr_arg++;
            }
            if (path==NULL) {
                printf("ERROR\ninvalid directory path\n");
                return -1;
            }
            printf("SUCCESS\n");
            return list(rec,path,end,perm);
        }
        else if(strcmp(argv[1],"parse")==0) {
        // s-a ales metoda parse si se verifica daca a fost trimis path-ul corect
            if (argc!=3)
                {
                    printf("ERROR\nNo path selected!\n");
                    return -1;
                }
            char *path = NULL;
            if (strncmp(argv[2],"path=",5)==0) 
                    path=argv[2]+5;
            else
                {
                    printf("ERROR\nInvalid path!\n");
                    return -1;
                }
            return parse(path);
        }
        else if(strcmp(argv[1],"extract")==0) {
            // s-a ales metoda extract si se verifica daca a fost trimis path-ul , linia si sectiunea;
            if (argc!=5)
                {
                    printf("ERROR\nArguments missing!\n");
                    return -1;
                }
            char *path = NULL;
            int line=-1;
            int section=-1;
            int nr_arg=2;
            while (nr_arg<5) {
                if (strncmp(argv[nr_arg],"path=",5)==0)
                    path=argv[nr_arg]+5;
                else if (strncmp(argv[nr_arg],"line=",5)==0) 
                    line=strtol(argv[nr_arg]+5,NULL,10);
                else if (strncmp(argv[nr_arg],"section=",8)==0)
                    section=strtol(argv[nr_arg]+8,NULL,10);
                nr_arg++;
            }
            if (path!=NULL && line!=-1 && section!=-1) 
                return extract(path,section,line);
        } 
        else if(strcmp(argv[1],"findall")==0) {
            // s-a ales metoda findall si se verifica daca a fost trimis path-ul 
            if (argc!=3)
                {
                    printf("ERROR\nNo path selected!\n");
                    return -1;
                }
            char *path = NULL;
            if (strncmp(argv[2],"path=",5)==0) 
                    path=argv[2]+5;
            else
                {
                    printf("ERROR\nInvalid path!\n");
                    return -1;
                }
            printf("SUCCESS\n");
            findall(path);
        }
    } else {
        printf("ERROR\nNo method selected!\n");
        return -1;
    }
}

int list(bool r, char* path , char* end, bool perm) {

    char * fname = (char*)malloc(1000*sizeof(char));
    DIR *dir = NULL;
	struct dirent *entry = NULL;
    struct stat statbuf;
    dir = opendir(path);
    if(dir == NULL) {
		printf("ERROR\nCould not open directory\n");
		return -1;
	}
    while((entry = readdir(dir)) != NULL) {
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name,"..")!=0) {
            //pt fiecare elemnt, se adauga calea completa 
		    snprintf(fname, 1000, "%s/%s", path, entry->d_name);
		    if(lstat(fname, &statbuf) == 0)  {
                if (r && S_ISDIR(statbuf.st_mode)) 
                    list(r,fname,end,perm); // daca s-a ales optiunea recursive si elementul este un director, se merge recursiv in fiecare subdirector
                if (end==NULL || strcmp(entry->d_name + (strlen(entry->d_name) - strlen(end)), end) == 0) {
                    if (perm==false ||  statbuf.st_mode & S_IWUSR) 
                            printf("%s\n", fname);// dupa verificarea ambelor filtre se afiseaza numele elementului
                }
		    } else {
			    printf("ERROR \nCan't get file stats\n");
                return -1;
            }
		}
	}
    free(fname);
    closedir(dir);
    return 0;
}

int parse (char *  path) {
    int fd;
    bool ok=true;
    if ((fd = open(path, O_RDONLY)) >= 0)
		{
            char s[5]={0};
            // se citeste si se verifica campul MAGIC care are 4 octeti
            if (read(fd, s, 4) == -1) {
                    printf("ERROR\n");
                    close(fd);
                    return -1;
            }
            //s[5]=0;
            if (strcmp(s, "0UZz")!=0) {
                printf("ERROR\nwrong magic");
                ok=false;
            }
            lseek(fd, 2, SEEK_CUR); // se sare peste campul header_size
            unsigned char v;
            // se citeste si se verifica campul VERSION care are un octet
            if (read(fd, &v, 1) == -1) {
                close(fd);
                return -1;
                }
            if (v < 50 || v > 121) {
                if(ok) {
                    ok=false;
                    printf("ERROR\nwrong version");
                }
                else
                    printf("|version");
            }
            unsigned char nS;
            // se citeste si se verifica numarul de sectiuni care are dim de un octet
            if (read(fd, &nS, 1) == -1) {
                close(fd);
                return -1;
                }
            if (nS < 7 || nS > 12) {
                if(ok) {
                    ok=false;
                    printf("ERROR\nwrong sect_nr");
                }
                else
                    printf("|sect_nr");
            }
            
            int nrSect = (int) nS;
            int i = 0;
            char t;
            // pt fiecare header al tututor sectiunilor se sare peste numele sectiunii de 9 octeti si se verifica section_type de 1 octet dupa care se sare 
            //peste cei 8 octeti de offset(4) si size(4)
            while (i < nrSect) {
                lseek(fd, 9, SEEK_CUR);
                if (read(fd, &t, 1) == -1) {
                    close(fd);
                    return -1;
                }
                if (t != 88 && t != 40) {
                    if(ok) {
                        ok=false;
                        printf("ERROR\nwrong sect_types");
                        break;
                    }
                    else {
                        printf("|sect_types");
                        break;
                    }
                }
                lseek(fd, 8, SEEK_CUR);
                i++;
            }
            if (!ok)
            {
                close(fd);
                return -1;
            }
            // daca s-a trecut peste toate verificarile, se afiseaza detaliile fisierului
            printf("SUCCESS\nversion=%d\nnr_sections=%d\n",v,nrSect);
            int fd2 = open(path, O_RDONLY);
            if (fd2<0) 
            {
                printf("ERROR\ninvalid file path\n");
                close(fd);
            }



            char* name = (char*)malloc(10*sizeof(char));
           
            int size = 0;
            lseek(fd2, 8, SEEK_SET);
            for (int i=0;i<nrSect;i++) {
                printf("section%d: ", i+1);
                if (read(fd2, name, 9) == -1) {
                    close(fd);
                    close(fd2);
                    printf("ERROR\n");
                    return -1;
                }
                name[9]=0;
                printf("%s ", name);



                if (read(fd2, &t, 1) == -1) {
                    close(fd);
                    close(fd2);
                    printf("ERROR\n");
                    return -1;
                }
                printf("%d ", t);
                lseek(fd2, 4, SEEK_CUR);
                if (read(fd2, &size, 4) == -1) {
                    close(fd);
                    close(fd2);
                    printf("ERROR\n");
                    return -1;
                }
                printf("%d", size);
                printf("\n");
            }
    free(name);
    close(fd);
    close(fd2);
    }
	else
		{
		    printf("ERROR\ninvalid file path\n");
            return -1;
		}

    return 0;
}

int extract (char *path, int section, int line) {
    int fd;
    bool ok=true;
    //se foloseste aceeasi verificare ca la functia parse daca fisierul este de tip SF sau nu
    if ((fd = open(path, O_RDONLY)) >= 0)
		{
            char s[5]={0};
            if (read(fd, s, 4) == -1) {
                    perror("ERROR\n");
                    close(fd);
                    return -1;
            }
            if (strcmp(s, "0UZz")!=0) {
                printf("ERROR\nwrong file");
                ok=false;
            }
            lseek(fd, 2, SEEK_CUR);
            unsigned char v;
            if (read(fd, & v, 1) == -1) {
                close(fd);
                return -1;
                }
            if (v < 50 || v > 121) {
                if(ok) {
                    ok=false;
                    printf("ERROR\nwrong file");
                }
            }
            unsigned char nS;
            if (read(fd, & nS, 1) == -1) {
                close(fd);
                return -1;
                }
            if (nS < 7 || nS > 12) {
                if(ok) {
                    ok=false;
                    printf("ERROR\nwrong file");
                }
                else
                    printf("|file");
            }
            
            int nrSect = (int) nS;
            int i = 0;
            char t;
            while (i < nrSect) {
                lseek(fd, 9, SEEK_CUR);
                if (read(fd, &t, 1) == -1) {
                    close(fd);
                    return -1;
                }
                if (t != 88 && t != 40) {
                    if(ok) {
                        ok=false;
                        printf("ERROR\nwrong file");
                        break;
                    }
                    else {
                        printf("|file");
                        break;
                    }
                }
                lseek(fd, 8, SEEK_CUR);
                i++;
            }
            close(fd);
            if (line<1) {
                if (ok)
                    {
                        printf("ERROR\nwrong line");
                        ok=false;
                    }
                else
                    printf("|line");
            }
            // daca fisierul este de tip SF , cautam linia dorita
            if (ok) {
                printf("SUCCESS\n");
                int fd2= open(path,O_RDONLY);
                if (fd2>=0) 
                {
                
                    lseek(fd2,8+(section-1)*18+10,SEEK_SET); // sarim peste cei 8 octeti de header si peste headerurile sectiunilor dinaintea sectiuni dorite ,
                    // cat si peste cei 10 octecti in care avem numele si tipul sectiuni dorite, ajungand astfel la offset-ul sectiunii noaste
                    int offset = 0;
                    int size= 0;
                    // se citesc offset si size pt sectiunea dorita
                    if (read(fd2, &offset, 4) == -1) {
                        close(fd2);
                        perror("ERROR\n");
                        return -1;
                    }
                   
                    if (read(fd2, & size, 4) == -1) {
                        close(fd2);
                        perror("ERROR\n");
                        return -1;
                    }
                    lseek(fd2,offset,SEEK_SET);// sarim la body ul sectiunii dorite 
                    char *fullSect = (char*)calloc((size),sizeof(char));
                    if (fullSect == NULL)
                    {
                        close(fd2);
                        perror("ERROR");
                        return -1;
                    }
                    if (read(fd2, fullSect, size) == -1) { // citim intreaga sectiune
                        free(fullSect);
                        close(fd2);
                        perror("ERROR\n");
                        return -1;
                    }
                    char* fullSectInv = (char*)calloc((size),sizeof(char));
                    if (fullSectInv == NULL)
                    {
                        free(fullSect);
                        close(fd2);
                        perror("ERROR");
                        return -1;
                    }
                    for (int i=size-1;i>=0;i--) 
                        fullSectInv[size-1-i]=fullSect[i]; // inversam fisierul, astfel ultima linie va fi linia 1 si fiecare linie va fi in ordine inversa
                    int nr_line=1;
                    int i;
                    for (i=0;nr_line!=line && i<size;i++) { // fiecare intalnire a octetului 0x0A reprezinta o linie noua, astfel parcurgem tot fisierul pana ajungem la linia dorita 
                        if (fullSectInv[i]==0x0A) 
                            nr_line++;
                    }
                    while (i<size && fullSectInv[i]!=0x0A) {
                        printf("%c",fullSectInv[i]);
                        i++;
                    }
                    printf("\n");
                    close(fd);
                    close(fd2);
                    free(fullSect);
                    free(fullSectInv);
                    return 0;
                }
                else
                {
                    close(fd);
                    printf("ERROR\ninvalid file path\n");
                    return -1;
                }
            }
            else {
                close(fd);
                return -1;
            }
        } else {
            printf("ERROR\ninvalid file path\n");
            return -1;
        }
    return 0;
}

int findall(char * path) {
    char * fname = (char*)malloc(1000*sizeof(char));
    DIR *dir = NULL;
	struct dirent *entry = NULL;
    struct stat statbuf;
    dir = opendir(path);
    if(dir == NULL) {
		printf("ERROR\nCould not open directory\n");
		return -1;
	}
    while((entry = readdir(dir)) != NULL) 
		if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name,"..")!=0) {
		    snprintf(fname, 1000, "%s/%s", path, entry->d_name); // pt fiecare element se adauga calea completa
		    if(lstat(fname, &statbuf) == 0)  {
                if (S_ISDIR(statbuf.st_mode)) 
                    findall(fname); //daca este un director se merge recursiv in toate subfoldere sale
                else { 
                int fd;
                bool ok=true;
                //se foloseste aceeasi verificare ca la functia parse daca fisierul este de tip SF sau nu
                if ((fd = open(fname, O_RDONLY)) >= 0)
		        {
                    char s[5]={0};
                    if (read(fd, s, 4) == -1) {
                        perror("ERROR\n");
                        close(fd);
                        return -1;
                    }
                    if (strcmp(s, "0UZz")!=0) 
                        continue;
                    lseek(fd, 2, SEEK_CUR);
                    unsigned char v;
                    if (read(fd, &v, 1) == -1) {
                        close(fd);
                        return -1;
                    }
                    if (v < 50 || v > 121) 
                        continue;
                    unsigned char nS;
                    if (read(fd, &nS, 1) == -1) {
                        close(fd);
                        return -1;
                    }
                    if (nS <7 || nS>12) 
                        continue;
                    int nrSect = (int) nS;
                    int i = 0;
                    char t;
                    while (i < nrSect) {
                        lseek(fd, 9, SEEK_CUR);
                        if (read(fd, &t, 1) == -1) {
                            close(fd);
                            return -1;
                        }
                        if (t != 88 && t != 40) {
                                ok=false;
                                break;
                            }
                        lseek(fd, 8, SEEK_CUR);
                        i++;
                    }
                    if (!ok)
                        continue;
                
                    int fd2 = open(fname,O_RDONLY);
                    if (fd2>=0) 
                    {   
                        int count=0;
                        // daca fisierul este de tip SF, se verifica fiecare sectiune daca are exact 13 linii
                        for (int i=0;i<nrSect;i++) {
                            lseek(fd2,8+i*18+10,SEEK_SET); // sarim peste cei 8 octeti de header si peste sectiunile dinainte sectiuni dorite ,
                            // cat si peste cei 10 octecti in care avem numele si tipul sectiuni dorite, ajungand astfel la offset-ul sectiunii noaste
                            int offset = 0;
                            int size= 0;
                            int lines=0;
                            if (read(fd2, &offset, 4) == -1) {
                                close(fd);
                                close(fd2);
                                perror("ERROR\n");
                                return -1;
                            }
                            if (read(fd2, &size, 4) == -1) {
                                close(fd);
                                close(fd2);
                                perror("ERROR\n");
                                return -1;
                            }
                            lseek(fd2,offset,SEEK_SET);// sarim la sectiunea dorita 
                            char *fullSect = (char*)malloc(size*sizeof(char));
                            if (read(fd2, fullSect, size) == -1) { // citim intreaga sectiune
                                perror("ERROR\n");
                                return -1;
                            }
                            // cand se intalneste octetul 0x0A se considera o linie noua
                            for(int i=0;i<size;i++) 
                                if (fullSect[i]==0x0A) 
                                     lines++;
                            //desi cerinta noastra este de 13 linii, se verifica la 12 deoarece ultima linie nu are la sfarsit octetul 0x0A
                            if (lines==12)
                                count++;      
                            free(fullSect);
                        }
                        // daca avem cel putin 4 sectiuni cu 13 linii, afisam elementul
                        if (count>=4)
                            printf("%s\n",fname);
                    } else {
                        close(fd);
                        printf("ERROR\ninvalid file path\n");
                        return -1;
                    }
                } else {
                    printf("ERROR\ninvalid file path\n");
                    return -1;
                }
                }
		    } else {
			    printf("ERROR \nCan't get file stats\n");
                return -1;
            }
		}
    free(fname);
    closedir(dir);
    return 0;
}
