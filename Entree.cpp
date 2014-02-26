/*************************************************************************
                           Entree  -  description
                             -------------------
    début                : Entree
    copyright            : (C) Entree par Entree
    e-mail               : Entree
*************************************************************************/

//---------- Réalisation de la tâche <Entree> (fichier Entree.cpp) ---

/////////////////////////////////////////////////////////////////  INCLUDE
//-------------------------------------------------------- Include système

//------------------------------------------------------ Include personnel
#include "Entree.h"
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques

//------------------------------------------------------ Fonctions privées
//static type nom ( liste de paramètres )
// Mode d'emploi :
//
// Contrat :
//
// Algorithme :
//
//{
//} //----- fin de nom
static int descR;
//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques
void cptFin(int noSignal){
	if(noSignal == SIGUSR2){
		exit(0);
	}
}

void Entree(TypeBarriere Parametrage){
	//Installation du handler
	struct sigaction action;

	action.sa_handler = cptFin ;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0 ;


	//armer sigusr2 sur cptFin;
	sigaction(SIGUSR2,&action,NULL);

	//Prof Blaise Pascal
	descR = open("fifo1",O_RDONLY);

	Voiture voiture;
	while(read(descR,&voiture,sizeof(Voiture))){
		// garage voiture ajout du pid voiturier dans la list
		pid_t voiturier=GarerVoiture(PROF_BLAISE_PASCAL);

	}

	close(descR);
	for(;;);
}


