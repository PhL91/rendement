/*
"define" ci-dessous nécessaire: sans lui, la compilation donne l'erreur:
warning: implicit declaration of function ‘strptime’
*/
#define _GNU_SOURCE
#define MAXOP 100	//Nombre maximum d'opérations
#define MAXCHAR 200	//Nombre maximum de caractères dans le nom de fichier operation
#define ITEMAX 2000	//Nombre d'itérations maximum pour le calcul du taux
#define CLINE 200	//Nombre de caractères max par ligne sur le fichier opération

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <libgen.h>
#include <stdbool.h>
#include <math.h>
#include <errno.h>
#include <fenv.h>

void usage(char *argv0) {
	printf("Utilisation: %s [OPTION]\n",basename(argv0));
	printf("  Calcul du rendement d'un placement en fonction des entrées/sorties de capitaux\n");
	printf("  -h,--help                             Affichage de l'aide\n");
	printf("  -o,--operation <fichier opération>    Défaut: operation.txt\n");
	printf("  -d,--date <date de clôture>           Défaut: aujourd'hui\n");
	printf("  -v,--valeur <valeur finale>           Défaut: sans mais obligatoire\n");
}

int main(int argc, char *argv[]) {

char fichieroperation[MAXCHAR]="operation.txt",*endptr,datefinchar[20],d[20],m[20],line[200];
FILE *pfichieroperation;
int option_index = 0,nop,nit,c,i,k;
double vf,vftmp,somme,tauxmin,tauxmax,tauxcur;
bool indicvf=false,indicdatefin=false;
struct tm tm,datefintm;
time_t u,datefinsec;
static struct option long_options[] = {
	{"operation",required_argument, NULL,  'o' },
	{"date",     required_argument, NULL,  'd' },
	{"valeur",     required_argument, NULL,  'v' },
	{"help",     no_argument,       NULL,  'h' },
	{0,          0,                 0,  0 }
};
struct ope {
	time_t datesec;
	struct tm datetm;
	char datechar[20];
	double montant;
	double interet;
	} ope[MAXOP]={0};

/* Elements de la ligne de commande */
while (1) {
	c = getopt_long(argc, argv, "ho:d:v:",long_options, &option_index);
	if (c == -1)
		break;
	switch (c) {
		case 0:
			/* Ne devrait jamais passer dans cette option car
			flag est toujours à NULL quelque soit l'option */
			printf("option %s", long_options[option_index].name);
			if (optarg) printf(" with arg %s\n", optarg);
			printf("\n");
			break;
		case 'h':
			usage(argv[0]);
			exit(EXIT_SUCCESS);
		case 'o':
			if (strlen(optarg) > MAXCHAR - 1) {
				printf("Le nombre de caractères du fichier operation excède la limite (%d)\n", MAXCHAR);
				exit(EXIT_FAILURE);
			}
			strcpy(fichieroperation,optarg);
			break;
		case 'd':
			if ( strptime(optarg,"%F",&datefintm) == 0 ) {
				printf("\nErreur sur strptime avec la date: %s\n",optarg);
				exit(EXIT_FAILURE);
			}
			strcpy(datefinchar,optarg);
			indicdatefin=true;
			break;
		case 'v':
			errno=0;
			vf=strtof(optarg,&endptr);
			if (errno != 0) {
				perror("strtof");
				exit(EXIT_FAILURE);
			}
			if (endptr == optarg) {
				fprintf(stderr, "Aucun chiffre trouvé\n");
				exit(EXIT_FAILURE);
			}
			if (*endptr != '\0') fprintf(stderr, "Incohérence: caractère(s) supplémentaire(s) sur valeur finale: \"%s\"\n", endptr);
			indicvf=true;
			break;
		case '?':
			printf("Erreur dans les options!\n");
			exit(EXIT_FAILURE);
		default:
			printf("Option non reconnue: 0%c\n", c);
			exit(EXIT_FAILURE);
	}
}
if (optind < argc) {
	fprintf(stderr, "Arguments inattendus sur la ligne de commande: ");
	while (optind < argc)
		fprintf(stderr,"%s ", argv[optind++]);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
}
if (!indicvf) {
	fprintf(stderr, "La valeur finale (-v ou --valeur) est obligatoire\n");
	exit(EXIT_FAILURE);
}
/*
Structure tm d'aujourd'hui ou à la date de simulation
*/
u=time(NULL);
tm = *localtime(&u);
if (!indicdatefin) {
	if (strftime(datefinchar,sizeof(datefinchar),"%F",&tm) == 0) {
		fprintf(stderr,"Erreur de conversion strftime\n");
		exit(EXIT_FAILURE);
	}
	datefintm=tm;
}
/* Les éléments tm_* doivent être initialisés avant l'appel à mktime()
sous peine de résultat aberrant. Je pense que c'est tm_isdst le premier responsable */
datefintm.tm_sec=0;
datefintm.tm_min=0;
datefintm.tm_hour=0;
datefintm.tm_isdst=0;
datefinsec=mktime(&datefintm);
if (datefinsec == (time_t)0) {
	perror("mktime");
}
pfichieroperation=fopen(fichieroperation,"r");
if (!pfichieroperation) {
	perror("fopen operation");
	exit(EXIT_FAILURE);
}
/* Mise en tableau des opérations */
k=0;
while (fgets(line,CLINE,pfichieroperation) != NULL) {
	if (strncmp("#",line,1) == 0) continue;
	if (sscanf(line,"%s %s",d,m) == EOF) {
		if (ferror(pfichieroperation) != 0) {
			perror("sscanf");
			exit(EXIT_FAILURE);
		}
	}
	if ( strptime(d,"%F",&ope[k].datetm) == 0 ) {
		printf("\nErreur sur strptime avec la date: %s\n",d);
		exit(EXIT_FAILURE);
	}
	strcpy(ope[k].datechar,d);
	ope[k].datetm.tm_isdst=0;
	ope[k].datesec=mktime(&ope[k].datetm);
	if (ope[k].datesec == (time_t)-1) {
		perror("mktime");
	}
	ope[k].montant=strtof(m,NULL);
	k=k+1;
	if (k > MAXOP) {
		printf("Nombre maximal d'opérations dépassé!\n");
		exit(EXIT_FAILURE);
	}
}
nop=k;
fclose(pfichieroperation);
/*
Calcul du taux initial:
	- Si la somme des montants placés est inférieur à la valeur finale, le rendement est positif: tauxmin=0 tauxcur=0.1 tauxmax=1000
	- Si la somme des montants placés est supérieur à la valeur finale, le rendement est négatif: tauxmin=-1 tauxcur=-0.1 tauxmax=0
*/
somme=0;
for (k=0;k<nop;k++) {
	somme=somme+ope[k].montant;
	}
if (somme < vf) {
	tauxmin=0;
	tauxcur=0.1;
	tauxmax=1000;
	}
else
	{
	tauxmin=-1;
	tauxcur=-0.1;
	tauxmax=0;
	}
/*
- Calcul du taux:
	- Boucle en calculant un montant itéré sur les opérations en prenant tauxcur comme taux de calcul
		- Si le montant itéré est supérieur à la valeur finale:
			- il faut baisser tauxcur: 
				- tauxmax=tauxcur
				- tauxcur=(tauxmin+tauxmax)/2
		- Si le montant itéré est inférieur à la valeur finale:
			- il faut monter tauxcur:
				- tauxmin=tauxcur
				- tauxcur=(tauxmin+tauxmax)/2
		- Prévoir un nombre d'itérations maximum
		- Arrêt de la boucle quand il y a moins de X% d'écart entre la somme finale donnée et la somme calculée
*/
k=1;
while (true) {
	somme=0;
	for (i=0;i<nop;i++) {
		errno=0;
		feclearexcept(FE_ALL_EXCEPT);
		vftmp=ope[i].montant*pow((1+tauxcur),((double)(datefinsec-ope[i].datesec)/(86400*365)));
		if (errno != 0 ) {
			fprintf(stderr,"Exception lors de l'appel de powf(3)\n");
			exit(EXIT_FAILURE);
			}
		ope[i].interet=vftmp-ope[i].montant;
		somme=somme+vftmp;
		}
	if (vf != 0) {
		if (fabs((somme-vf)/vf) < 0.00001) break;
		}
	else
		if (somme != 0)
			{
			if (fabs((somme-vf)/somme) < 0.00001) break;
			}
		else
			{
			if (fabs(somme-vf) < 0.001) break;
			}
	if (somme > vf) {
		tauxmax=tauxcur;
		tauxcur=(tauxmin+tauxmax)/2;
		}
	else
		{
		tauxmin=tauxcur;
		tauxcur=(tauxmin+tauxmax)/2;
		}
	k=k+1;
	if ( k > ITEMAX) {
		fprintf(stderr,"Nombre maximum d'itérations (%d) dépassé\n",ITEMAX);
		fprintf(stderr,"Les valeurs seront à apprécier!\n");
		k=k-1;
		break;
		}
	}
nit=k;
/*
Impression des résultats
*/
printf("Nombre d'opérations: %d\n",nop);
printf("Valeur finale: %#10.2f\n",vf);
printf("Date d'estimation de la rentabilité: %s\n",datefinchar);
printf("\n\n");
printf("                        Nombre de\n");
printf("      Date    Montant    jours de   Intérêts\n");
printf(" opération  opération   placement    générés\n");
somme=0;
for (k=0;k<nop;k++) {
	printf("%s %#10.2f %11d %#10.2f\n",ope[k].datechar,ope[k].montant,(int)((datefinsec-ope[k].datesec)/86400),ope[k].interet);
	somme=somme+ope[k].montant+ope[k].interet;
	}
printf("\nTaux calculé(%%):                            %-#10.4f\n",tauxcur*100);
printf("Valeur finale après calcul taux:            %-#10.2f\n",somme);
printf("Nombre d'itérations pour le calcul du taux: %-d\n",nit);

exit(EXIT_SUCCESS);
}

