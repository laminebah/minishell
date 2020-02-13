#ifndef _H_EXEC_H
#define _H_EXEC_H
#include "readcmd.h"

typedef struct{
	int numero;
	char * nameJob;
	int statut;//0: en cours , 1: stopped et 2 : terminé
	pid_t pid;
}jobs;

jobs ** tableJob;

/**
* \file  executeCmd.h
* \brief Gestion de l'exécution de toutes les commandes implémentées dans le shell
*/

/**
 * \brief      Fonction pour le traitement des signaux
 * \details    En Fonction du signal reçu(SIGINT,SIGTSTP,SIGCHLD),
 							 un traitant de signal est défini pour le gérer
							 spécifiquement
 * \param      signal_recu un entier
 */
void traite_signal(int signal_recu) ;

/**
 * \brief      Fonction pour l'exécution des commandes simples
 * \details    Utilisation des entrées stockées dans le champ
 							 seq de la structure cmdline.fonction modifiée à
							 chaque nouvelle fonctionnalité
 * \param      cmd une structure de cmdline
 */
void executer_commandes(struct cmdline * cmd);

/**
 * \brief      Fonction pour l'exécution des commandes pipelinées
 * \details		 plusieurs cas sont traités dans cette fonction
 							 -si on a plusieurs commandes
							 -si redirection en entrée sur 1ère commande
							 -si c'est la 1ère commande
							 -si c'est le dernier pipe
							 -sinon pour tout autre cas sur les pipes
 * \param      numCmd le numéro de la commande
 * \param      cmd une structure de commande
 * \param      tabDesc un tableau d'entier
 */
pid_t executer_commande_dans_un_fils(int numCmd,struct cmdline * cmd,int ** tabDesc);

/**
 * \brief      Fonction pour quitter le shell
 * \details    sort du shell si "q","quit"ou"Q" a été tapé
 * \param      l une structure de commande
 */
void quitter (struct cmdline *l);

/**
 * \brief      Fonction pour l'ajout des commandes en arrière plan
 							 dans un tableau
 * \param      numero un entier
 * \param      nameJob le nom de la commande
 * \param      statut (0:en cours d'execution,1:stoppé,2:terminé)
 * \param      pid le pid de la commande
 * \param      tableJob le tableau de commande où ajouter
 */
void ajoutJob (int numero,char * nameJob,int statut,pid_t pid,jobs **tableJob);

/**
 * \brief      Fonction d'affichage des informations du tableau après
 							 la saisie de la commande jobs
 * \details    Si des taches en arrière plan avaient été lancées,
  						 affiche par ordre le numéro,le pid,le statut(en cours d'execution,
						 	 stoppé,terminé) et le nom des commandes tapées
 * \param      tableJob le tableau des commandes en arrière plan
 */
void afficheJobs (jobs **tableJob);

/**
 * \brief      Fonction pour l'exécution de la commande fg
 * \details		 si des commandes ont été lancées en arrière plan
 							 le dernier processus est tué
							 on l'exécute à nouveau en 1er plan
							 puis l'enlève de tableJob
 * \param			 cmd une structure de cmdline
 * \param			 tableJob le tableau de commandes en arrière plan
 */
void cmdFg (struct cmdline * cmd,jobs **tableJob);

/**
 * \brief      Fonction pour l'exécution de la commande stop
 * \details		 on teste s'il y'a une commande en arrière plan
 							 puis le tue si existante
 * \param	     tableJob le tableau de commande en arrière plan
 */
void cmdStop(jobs ** tableJob);
#endif
