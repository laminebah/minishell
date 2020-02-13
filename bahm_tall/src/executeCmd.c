#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "readcmd.h"
#include "csapp.h"
#include <signal.h>
#include "executeCmd.h"
#define RUNNING 0
#define STOPPED 1
#define TERMINATED 2
pid_t globalPID = 0;
int nb_job = 0;
char nomCmd[50];

void traite_signal(int signal_recu) {
    switch (signal_recu) {
  	/* Si SIGINT et que globalPID est défini alors
  			envoi du signal SIGINT au PID*/
      case SIGINT :
        printf("\nshell> ");
        fflush(stdout);
  		  if(globalPID>0) {
          kill(globalPID,SIGINT);
        }
        break;

  	/* Si SIGTSTP et que globalPID est défini alors
  			envoi du signal SIGTSTP au PID et ajout de la commande
        avec comme statut stoppé dans la table des jobs
        après une frappe de ctrlZ*/
    	case SIGTSTP :
    		if(globalPID>0) {
            nb_job += 1;
            ajoutJob (nb_job,nomCmd,STOPPED,globalPID,tableJob);
            kill(globalPID,SIGTSTP);
            printf ("\n");
            fflush (stdout);
        }
    		break;

  	/* Si SIGCHLD on n'attend pas le processus*/
    	case SIGCHLD :
    		while(waitpid(-1,NULL, WNOHANG) > 0);
    		break;
        	default:
              printf("Signal inattendu\n");
    }
}


void quitter (struct cmdline *l) {
	if(!strcmp(l->seq[0][0],"quit") || !strcmp(l->seq[0][0],"Q") || !strcmp(l->seq[0][0],"q")){
		exit(0);
		}
}


void executer_commandes(struct cmdline * cmd){
	if(!(cmd->seq[1] == NULL && cmd->bg == WNOHANG)){
    strcpy(nomCmd,cmd->seq[0][0]);
		pid_t pidFils;
		int **tabDesc = malloc(sizeof(int *)*cmd->nbCmd-1); // Tableau de pipes
		for (int i = 0; i < cmd->nbCmd; i++) {
				// on lance l'exécution de la commande dans un fils
				pidFils =executer_commande_dans_un_fils(i,cmd,tabDesc);
		}
		// Le père attend la mort de tous ses fils
		waitpid(pidFils,NULL,WUNTRACED);

		//On libère le tableau de pipes
		for (int j=0; j<cmd->nbCmd-1; j++)
				free(tabDesc[j]);
		free(tabDesc);
 } else {
		pid_t pid = fork ();
		if (pid == 0) {
			execvp(cmd->seq[0][0],cmd->seq[0]);
		} else {
			waitpid(-1,NULL,WNOHANG);
			nb_job += 1;
			ajoutJob(nb_job,cmd->seq[0][0],RUNNING,pid,tableJob);
		}
 }
}

pid_t executer_commande_dans_un_fils(int numCmd,struct cmdline * cmd,int ** tabDesc){
	pid_t pidFils;
	// Si on a plusieurs commandes
	if (numCmd != cmd->nbCmd - 1) {
		// création d'un pipe dans le tableau de pipes
		tabDesc[numCmd] = malloc(sizeof(int)*2);
        if (pipe(tabDesc[numCmd]) == -1) {
            perror("Echec création tube");
            exit(1);
        }
 }
 // Création d'un fils pour exécuter la commande
 if ((pidFils = fork()) == 0){
	 // Si on a une redirection en entrée sur la première commande
	 if( numCmd==0 && cmd->in != NULL ){
		 int fd = open(cmd->in, O_RDONLY,S_IRUSR | S_IWUSR);
		 if (fd < 0) {
	 			if (errno == EACCES)
	 				printf ("shell>  %s: Permmission non accordée\n",cmd->in);
	 			else
	 				printf ("shell>  %s: Aucun fichier ou dossier de ce type\n",cmd->in);
	 			exit (1);
	 	}
		 if(dup2(fd, STDIN_FILENO)<0){
			 printf("Erreur sur le dup2");
			 exit(1);
		 }
		 close(fd);
	 }
	 /* Si c'est la première commande, on redirige la sortie
	 		à l'entrée du pipe*/
	 if(numCmd == 0){
		 if(numCmd != cmd->nbCmd - 1){
			 dup2(tabDesc[numCmd][1], STDOUT_FILENO);
			 close(tabDesc[numCmd][1]);
			 close(tabDesc[numCmd][0]);
		 }
	/* Si c'est le dernier pipe alors on redirige uniquement
	 	 la sortie du pipe précédent sur son entrée */
	 }else if(numCmd ==  cmd->nbCmd - 1){
		 dup2(tabDesc[numCmd-1][0],STDIN_FILENO);
		 close(tabDesc[numCmd-1][1]);
		 close(tabDesc[numCmd-1][0]);
	/* Sinon on redirige l'entrée et sortie des commandes
	 	avec le pipe précedent et le pipe actuel */
	 }else{
		 dup2(tabDesc[numCmd][1], STDOUT_FILENO);
		 dup2(tabDesc[numCmd-1][0], STDIN_FILENO);
		 close(tabDesc[numCmd-1][1]);
		 close(tabDesc[numCmd-1][0]);
		 close(tabDesc[numCmd][1]);
		 close(tabDesc[numCmd][0]);
	}
/*Si on est la dernière commande et qu'il y a une redirection
	en sortie */
	if(numCmd ==  cmd->nbCmd-1 && cmd->out != NULL){
		errno = 0;
		int fd = open(cmd->out,O_WRONLY | O_CREAT | O_TRUNC ,S_IRUSR | S_IWUSR);
		if (fd < 0) {
			if (errno == EACCES)
				printf ("shell>  %s: Permmission non accordée\n",cmd->out);
			else
				printf ("shell>  %s: Erreur création du fichier jhh\n",cmd->out);
			exit (1);
		}
		if(dup2(fd, STDOUT_FILENO)<0){
			printf("Erreur sur le dup2 ");
			exit(1);
		}
		close(fd);
	}
	/*on indique que le gestionnaire par defaut du signal doit etre installé */
	signal(SIGINT,SIG_DFL);
	signal(SIGTSTP,SIG_DFL);
/* On execute la commande*/
	if (execvp(cmd->seq[numCmd][0],cmd->seq[numCmd]) < 0){
		printf("%s : commande introuvable\n",cmd->seq[0][0]);
		exit(1);
	}
/* Dans le père on ferme les pipes précédents au fur et à mesure
	 si on n'est pas la première commande*/
 }else{
	 if(numCmd!=0){
		 close(tabDesc[numCmd-1][1]);
		 close(tabDesc[numCmd-1][0]);
	 }
 }
 globalPID = pidFils;
 return pidFils;
}
void ajoutJob(int numero,char * nameJob,int statut,pid_t pid,jobs ** tableJob) {
	tableJob [nb_job - 1] = malloc (sizeof(jobs));
	tableJob [nb_job - 1] -> numero = numero;
	tableJob [nb_job - 1] -> nameJob = malloc (sizeof(char)*strlen(nameJob));
	strcpy (tableJob [nb_job - 1] -> nameJob,nameJob);
	tableJob [nb_job - 1] -> statut = statut;
	tableJob [nb_job - 1] -> pid = pid;

	//free (job);
}
void afficheJobs (jobs **tableJob){
	//printf ("%d\n",nb_job);
		for(int i=0;i<nb_job;i++){
			printf("[%d]\t",tableJob[i]->numero);
			printf("%d\t",tableJob[i]->pid);
			switch (tableJob[i]->statut){
				case RUNNING :
						printf("En cours d'exécution\t"); break;
				case STOPPED :
						printf("Arrếté \t"); break;
				case TERMINATED :
						printf("Exécution terminée\t"); break;
			}
			printf("%s\n",tableJob[i]->nameJob);
		}
}
void cmdFg (struct cmdline * cmd,jobs **tableJob) {
	//aucune commande n'est lancée arrière plan
  int status;
	if (nb_job == 0)
		fprintf(stderr,"shell>: %s: courant: tache inexistante\n",cmd->seq[0][0]);
	else {
  		Kill (tableJob[nb_job - 1]->pid,SIGTERM);
  		char **ncmd = malloc (sizeof(char *)*2);
  		//on exécute à nouveau la commande en premier plan
  		strcpy (ncmd[0],tableJob [nb_job - 1]->nameJob);
  		ncmd [1] = NULL;
  		//on efface cette commande de  la table des jobs
  		free (tableJob [nb_job - 1]);
  		nb_job --;
  		//printf ("%s\n",cmd[0]);
  		pid_t pid = Fork ();
  		if (pid == 0){
  			//on exécute la commande au premier plan
  			errno = 0;
  			if (execvp (ncmd[0],ncmd) < 0) {
  				fprintf( stderr,"%s: commande introuvable\n",ncmd[0]);
  		  		exit (errno);//en cas d'erreur si l'exécution échoue
  			}
  		} else {
  			waitpid(pid,&status,0);
  		}

	}
}
void cmdStop(jobs ** tableJob) {
  if(nb_job!=0){
    int i = nb_job - 1;
    while (i >= 0 && tableJob [i]->statut != RUNNING)
      i ++;
    if (i < 0)
      printf ("Toutes les commandes sont soient arrêtées out stoppées déjà\n");
    else {
      tableJob [i] -> statut = STOPPED;
      kill (tableJob [i]->pid,SIGTSTP);
    }
  }
}
