/*
 * Copyright (C) 2002, Simon Nieuviarts
 */
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

int main(){
	// Redéfinition des handlers
	signal(SIGINT,traite_signal);
	signal(SIGTSTP,traite_signal);
	signal(SIGCHLD,traite_signal);
	tableJob = (jobs **) malloc (sizeof(jobs *)*30);
	while (1) {
		struct cmdline *l;
		printf("shell> ");
		l = readcmd();
		/* If input stream closed, normal termination */
		if (!l) {
			printf("exit\n");
			exit(0);
		}
		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}
		  quitter(l);
			if (!strcmp(l->seq[0][0],"jobs")) {
				//fonction d'affichage jobs
				afficheJobs (tableJob);
			}
			else if (!strcmp(l->seq[0][0],"fg")){
				//ramené au premier plan
				cmdFg (l,tableJob);
			} else if (!strcmp(l->seq[0][0],"stop")){
				//commande stop
			  cmdStop(tableJob);
			} else
				executer_commandes(l);
	}
	for(int i=0;tableJob[i];i++){
		free(tableJob[i]);
	}
	free(tableJob);
}
