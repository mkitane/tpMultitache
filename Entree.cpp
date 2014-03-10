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
#include <sys/wait.h>
#include <iostream>
#include <map>
#include <stdio.h>
#include <string.h>


#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
///////////////////////////////////////////////////////////////////  PRIVE
//------------------------------------------------------------- Constantes

//------------------------------------------------------------------ Types

//---------------------------------------------------- Variables statiques
static int descR;
static map<pid_t,Voiture> mapVoiture;
static int memID;
static int semID;
//------------------------------------------------------ Fonctions privées
static void init(TypeBarriere Parametrage);
static void moteur(TypeBarriere Parametrage);
static void destruction(int noSignal);
static void receptionMortVoiturier(int noSignal);


static void init(TypeBarriere Parametrage){
	//Installation du handler destruction
	struct sigaction action;
	action.sa_handler = destruction ;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0 ;
	//armer sigusr2 sur handlerEntree;
	sigaction(SIGUSR2,&action,NULL);


	//Installation du handler destruction
	struct sigaction actionFinVoiturier;
	actionFinVoiturier.sa_handler = receptionMortVoiturier ;
	sigemptyset(&actionFinVoiturier.sa_mask);
	actionFinVoiturier.sa_flags = 0 ;
	sigaction(SIGCHLD,&actionFinVoiturier,NULL);

	switch(Parametrage)
	{
		case(PROF_BLAISE_PASCAL):
			//Prof Blaise Pascal
			descR = open(CANAL_PROF_BP,O_RDONLY);
			break;
		case(AUTRE_BLAISE_PASCAL):
			descR = open(CANAL_AUTRE_BP,O_RDONLY);
			break;
		case(ENTREE_GASTON_BERGER):
			descR = open(CANAL_GB,O_RDONLY);
			break;
		default:
			break;
	}
}

static void moteur(TypeBarriere Parametrage)
{
	Voiture voiture;
	struct sembuf reserver = {MutexMP, -1,0};	//p Operation --> Reservation
	struct sembuf liberer = {MutexMP, 1, 0};	//v Operation --> liberation

		if(read(descR,&voiture,sizeof(Voiture)) > 0){

			DessinerVoitureBarriere(Parametrage,voiture.TypeUsager);

			if( semctl(semID,SemaphoreCompteurPlaces,GETVAL,0) <= 0){
				//On place en liste d'attente !
				AfficherRequete(Parametrage, voiture.TypeUsager, voiture.heureArrivee);


				//On ecrit dans la mémoire partagée que l'on a une requete !


				while(semop(semID,&reserver,1)==-1 && errno==EINTR); //Reservation de la memoire partagee

				//Ecrire la voiture sur la mémoire partagée
				memStruct *a = (memStruct *) shmat(memID, NULL, 0) ;
				a->requetes[Parametrage-1] =  voiture ;
				shmdt(a);


				semop(semID,&liberer,1); //Liberation de la memoire partagee




				struct sembuf pOp = {Parametrage,-1,0};  //p Operation sur le mutex de synchronisation
				while(semop(semID,&pOp,1)==-1 && errno==EINTR);

				Effacer((TypeZone)(ETAT_P8+Parametrage));
			}


			//On met a jour le Semaphore compteur de place
			struct sembuf pOp = {SemaphoreCompteurPlaces,-1,0};
			semop(semID,&pOp,1);




			// garage voiture ajout du pid voiturier dans la list
			pid_t voiturier=GarerVoiture(Parametrage);
			mapVoiture.insert(pair<pid_t,Voiture>(voiturier,voiture));

			//sleep 1s
			sleep(TEMPO);

		}
}

static void destruction(int noSignal)
{
	if(noSignal == SIGUSR2){
		//On masque SIGCHLD avant de killer !
		struct sigaction action;
		action.sa_handler = SIG_IGN ;
		sigemptyset(&action.sa_mask);
		action.sa_flags = 0 ;
		//armer sigusr2 sur handlerEntree;
		sigaction(SIGCHLD,&action,NULL);



		for(map<pid_t,Voiture>::iterator it=mapVoiture.begin(); it!= mapVoiture.end() ; it++){
			kill(it->first,SIGUSR2);
		}
		for(map<pid_t,Voiture>::iterator it=mapVoiture.begin(); it!= mapVoiture.end() ; it++){
			waitpid(it->first,NULL,0);
		}

		close(descR);
		exit(0);
	}
}

static void receptionMortVoiturier(int noSignal)
{

	if(noSignal == SIGCHLD){
		struct sembuf reserver = {MutexMP, -1,0};	//p Operation --> Reservation
		struct sembuf liberer = {MutexMP, 1, 0};	//v Operation --> liberation


		int status;
		//Recuperer le fils qui a envoye le SIGCHLD
		pid_t filsFini = wait(&status);


		//Recuperer la bonne voiture qui a lancé le signal
		map<pid_t,Voiture>::iterator itLE = mapVoiture.find(filsFini);
		Voiture v = itLE ->second ;

		//Afficher ses caractéristiques dans l'endroit indique
		AfficherPlace(WEXITSTATUS(status),v.TypeUsager,v.numeroVoiture,v.heureArrivee);




		while(semop(semID,&reserver,1)==-1 && errno==EINTR);


		//Ecrire la voiture sur la mémoire partagée
		memStruct *a = (memStruct *) shmat(memID, NULL, 0) ;
		a->voituresPartagee[WEXITSTATUS(status)-1] = v ;
		shmdt(a);



		semop(semID,&liberer,1);




		//Supprimer la bonne voiture de la map des voitures en train de stationner
		mapVoiture.erase(itLE);

	}
}
//////////////////////////////////////////////////////////////////  PUBLIC
//---------------------------------------------------- Fonctions publiques


void Entree(TypeBarriere Parametrage,int pmemID, int psemID){
	memID = pmemID;
	semID = psemID;

	init(Parametrage);

	for(;;){
		moteur(Parametrage);
	}
}


