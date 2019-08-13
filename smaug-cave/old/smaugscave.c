#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h>

//funciton to end simulation:

int SmaugDeath();
void* generateThieves();


int maximumThiefInterval;
int maximumHunterInterval;
int winProb;
int smaugJewelsFD;
int playCountFD;
int fightCountFD;
int thiefCountFD;
int hunterCountFD;
int thiefCanContinueFD;
int hunterCanContinueFD;
int defeatedHuntersCountFD;
int defeatedThievesCountFD;
int smaugActiveFD;

sem_t *play;
sem_t *fight;
sem_t *thiefLeaving;
sem_t *hunterLeaving;
sem_t *thiefCanContinue;
sem_t *hunterCanContinue;
//sem_t *smaugJewelsMutex;
sem_t *playCountMutex;
sem_t *fightCountMutex;
sem_t *thiefCountMutex;
sem_t *hunterCountMutex;
sem_t *thiefCanContinueCountMutex;
sem_t *hunterCanContinueCountMutex;
sem_t *smaugJewelCounterMutex;
sem_t *defeatedHuntersCountMutex;
sem_t *defeatedThievesCountMutex;
sem_t *smaugActiveMutex;
sem_t *smaugActive;

void *smaugJewels;
void *playCount;
void *fightCount;
void *thiefCount;
void *hunterCount;
void *thiefCanContinueCount;
void *hunterCanContinueCount;
void *defeatedHuntersPTR;
void *defeatedThievesPTR;
void *smaugActivePTR;


const int initialJewelCount = 30;
const int smaugJewelCounterSize = 4; // bytes
const int size = 4;
const int initialValue = 0;
const char *smaugJewelsCountName = "/smaugsJewelsInTreasure"; // for Smaug's Jewels in Treasure counter
const char *thiefPlay = "/thiefWaitLine"; // for line-up of thieves waiting to play with Smaug
const char *hunterFight = "/hunterWaitLine"; // for line-up of hunters waiting to fight Smaug
const char *thiefDone = "/thiefTermination"; // for Smaug to know process will terminate
const char *hunterDone = "/hunterTermination"; // for Smaug to know process will terminate
const char *thiefCanContinueName = "/thiefContinues";
const char *hunterCanContinueName = "/hunterContinues";
const char *thiefCanContinueCountName = "/theifCanContinue";
const char *hunterCanContinueCountName = "/hunterCanContinue";
const char *playCountName = "/thievesReadyToPlay";
const char *fightCountName = "/huntersReadyToFight";
const char *thiefCountName = "/thievesCount";
const char *hunterCountName = "/hunterCount";
const char *playCountMutexName = "/playMutex";
const char *fightCountMutexName = "/fightMutex";
const char *thiefCountMutexName = "/thiefMutex";
const char *hunterCountMutexName = "/hunterMutex";
const char *thiefCanContinueCountMutexName = "/thiefContinueMutex";
const char *hunterCanContinueCountMutexName = "/hunterContinueMutex";
const char *smaugJewelCounterMutexName = "/smaugsJewelCounterMutex";
const char *defeatedHuntersCountMutexName = "/numDefeatedHuntersMutex";
const char *defeatedThievesCountMutexName =  "/numDefeatedThievesMutex";
const char *defeatedHuntersCountName = "/numDefeatedHunters";
const char *defeatedThievesCountName = "/numDefeatedThieves";
const char *smaugActiveMutexName = "/smaugActiveMutex";
const char *smaugActiveName = "/smaugActive";

const int ONESECOND=1000000;



int main() {
//initialize values:
	maximumThiefInterval = 0;
	maximumHunterInterval = 0;
	winProb = 0;

//read paramaters from user input:
	 printf("Please input the maximum time between creation of thieves\n ");
	 scanf("%d", &maximumThiefInterval);
	 printf("Please input the maximum time between creation of treasure hunters\n ");
	 scanf("%d", &maximumHunterInterval);
	 printf("Please input the probability of a thief or treasure hunter not being defeated by Smaug\n ");
	 scanf("%d", &winProb);


 //create Smaug process:
 	pid_t	smaug_id = fork();
 	if(smaug_id < 0){
 			perror("fork");
 			exit(EXIT_FAILURE);
	}

	if (smaug_id=0){
			//initializations:

			play = sem_open(thiefPlay, O_CREAT, 0600, 0);
			if(play == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			fight = sem_open(hunterFight, O_CREAT, 0600, 0);
			if(fight == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			thiefLeaving = sem_open(thiefDone, O_CREAT, 0600, 0);
			if(thiefLeaving == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			hunterLeaving = sem_open(hunterDone, O_CREAT, 0600, 0);
			if(hunterLeaving == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			thiefCanContinue = sem_open(thiefCanContinueName, O_CREAT, 0600, 1);//NOTE: initialize to 0 or 1?
			if(thiefCanContinue == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			hunterCanContinue = sem_open(hunterCanContinueName, O_CREAT, 0600, 1); //NOTE: initialize to 0 or 1?
			if(hunterCanContinue == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			playCountMutex = sem_open(playCountMutexName,  O_CREAT, 0600, 1);
			if(playCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			fightCountMutex = sem_open(fightCountMutexName,  O_CREAT, 0600, 1);
			if(fightCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			thiefCountMutex = sem_open(thiefCountMutexName,  O_CREAT, 0600, 1);
			if(thiefCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			hunterCountMutex = sem_open(hunterCountMutexName,  O_CREAT, 0600, 1);
			if(hunterCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			thiefCanContinueCountMutex = sem_open(thiefCanContinueCountMutexName,  O_CREAT, 0600, 1);
			if(thiefCanContinueCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			hunterCanContinueCountMutex = sem_open(hunterCanContinueCountMutexName,  O_CREAT, 0600, 1);
			if(hunterCanContinueCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			smaugJewelCounterMutex = sem_open(smaugJewelCounterMutexName,  O_CREAT, 0600, 1);
			if(smaugJewelCounterMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			defeatedHuntersCountMutex = sem_open(defeatedHuntersCountMutexName,  O_CREAT, 0600, 1);
			if(defeatedHuntersCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			defeatedThievesCountMutex = sem_open(defeatedThievesCountMutexName,  O_CREAT, 0600, 1);
			if(defeatedThievesCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}
			smaugActiveMutex = sem_open(smaugActiveMutexName,  O_CREAT, 0600, 1);
			if(smaugActiveMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

		//TODO: check for errors in opening for these:

			smaugJewelsFD = shm_open(smaugJewelsCountName, O_CREAT | O_RDONLY, 0666);
			smaugJewels = mmap(0, smaugJewelCounterSize , PROT_READ | PROT_WRITE, MAP_SHARED, smaugJewelsFD, 0);
			memcpy(smaugJewels, initialJewelCount, sizeof(initialJewelCount)); // to copy the value into this shared counter

			playCountFD = shm_open(playCountName, O_CREAT | O_RDONLY, 0666);
			playCount = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, playCountFD, 0);
			memcpy(playCount, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			fightCountFD = shm_open(fightCountName, O_CREAT | O_RDONLY, 0666);
			fightCount = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, fightCountFD, 0);
			memcpy(fightCount, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			thiefCountFD = shm_open(thiefCountName, O_CREAT | O_RDONLY, 0666);
			thiefCount = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, thiefCountFD, 0);
			memcpy(thiefCount, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			hunterCountFD = shm_open(hunterCountName, O_CREAT | O_RDONLY, 0666);
			hunterCount = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, hunterCountFD, 0);
			memcpy(hunterCount, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			thiefCanContinueFD = shm_open(thiefCanContinueCountName, O_CREAT | O_RDONLY, 0666);
			thiefCanContinueCount = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, thiefCanContinueFD, 0);
			memcpy(thiefCountName, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			hunterCanContinueFD = shm_open(hunterCanContinueCountName, O_CREAT | O_RDONLY, 0666);
			hunterCanContinueCount = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, hunterCanContinueFD, 0);
			memcpy(hunterCountName, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			defeatedHuntersCountFD = shm_open(defeatedHuntersCountName, O_CREAT | O_RDONLY, 0666);
			defeatedHuntersPTR = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, defeatedHuntersCountFD, 0);
			memcpy(defeatedHuntersCountName, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			defeatedThievesCountFD = shm_open(defeatedThievesCountName, O_CREAT | O_RDONLY, 0666);
			defeatedThievesPTR = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, defeatedThievesCountFD, 0);
			memcpy(defeatedThievesCountName, initialValue, sizeof(initialValue)); // to copy the value into this shared counter

			smaugActive = shm_open(smaugActiveName, O_CREAT | O_RDONLY, 0666);
			smaugActivePTR = mmap(0, size , PROT_READ | PROT_WRITE, MAP_SHARED, smaugActive, 0);
			memcpy(smaugActiveName, initialValue, sizeof(initialValue)); // to copy the value into this shared counter


			//execution of Smaug's role:
			int numThievesWaiting = 0;
			int numHuntersWaiting = 0;
			void* nTW;
			void* nHW = 0;

			while(1){
				//this is supposed to execute whats given below in comments
				sem_wait(playCountMutex); // already a pointer, no need to do &
				memcpy(nTW, playCountName, sizeof(playCountName));
				numThievesWaiting = *((int*)(nTW));
				sem_post(playCountMutex);

				sem_wait(fightCountMutex);
				memcpy(nHW, fightCountName, sizeof(fightCountName));
				numHuntersWaiting = *((int*)(nHW));
				sem_post(fightCountMutex);

				while((numThievesWaiting) > 0 || (numHuntersWaiting > 0)){
					//do all the important stuff - playing and fighting

					while(numThievesWaiting > 0){
						//allow thief to play:
						sem_post(play);
						//playing with thief - thief will update counter for numThieves i think

						//Smaug will wait until the thief is all done:
						sem_wait(thiefLeaving);

						//check if Smaug dies:
						if (SmaugDeath()) {
							break;
						}
						//add function to do this

						//repeat check for numthieves here:
						memcpy(nTW, playCountName, sizeof(playCountName));
						numThievesWaiting = *((int*)(nTW));
						sem_post(playCountMutex);

					}

					if(numHuntersWaiting > 0){
						//allow hunter to fight:
						sem_post(fight);

						//playing with thief - thief will update counter for numThieves i think

						//Smaug will wait until the thief is all done:
						sem_wait(hunterLeaving);


						//check if Smaug dies:
						// SmaugDeath(); //ifk if unneccesary

						//repeat check for numHunters here:
						sem_wait(fightCountMutex);
						memcpy(nHW, fightCountName, sizeof(fightCountName));
						numHuntersWaiting = *((int*)(nHW));
						sem_post(fightCountMutex);

					}

					//check if Smaug dies:
					if (SmaugDeath()) {
						break;
					}

					//REPEAT CHECK FOR NUM PEOPLE IN HERE TOO - uncomment below when know where to put basically

					sem_wait(playCountMutex); // already a pointer, no need to do &
					memcpy(nTW, playCountName, sizeof(playCountName));
					numThievesWaiting = *((int*)(nTW));
					sem_post(playCountMutex);

					sem_wait(fightCountMutex);
					memcpy(nHW, fightCountName, sizeof(fightCountName));
					numHuntersWaiting = *((int*)(nHW));
					sem_post(fightCountMutex);

				}

			}

			//clean-up:
			if (sem_close(play) != 0){
					perror("Parent  : [sem_close] for play Failed\n");
					return  3;
			}
			if( sem_unlink(thiefPlay) < 0 ){
					printf("Parent  : [sem_unlink] for thiefPlay Failed\n");
					return 6;
			}

			if (sem_close(fight) != 0){
					perror("Parent  : [sem_close] for fight Failed\n");
					return  3;
			}
			if( sem_unlink(hunterFight) < 0 ){
					printf("Parent  : [sem_unlink] for hunterFight Failed\n");
					return 6;
			}

			if (sem_close(thiefLeaving) != 0){
					perror("Parent  : [sem_close] for thiefLeaving Failed\n");
					return  3;
			 }
			 if( sem_unlink(thiefDone) < 0 ){
					printf("Parent  : [sem_unlink] for thiefDone Failed\n");
					return 6;
			 }

			 if (sem_close(hunterLeaving) != 0){
					perror("Parent  : [sem_close] for hunterLeaving Failed\n");
					return  3;
			 }
			 if( sem_unlink(hunterDone) < 0 ){
					printf("Parent  : [sem_unlink] for hunterDone Failed\n");
					return 6;
			 }

			 if (sem_close(hunterCanContinue) != 0){
					perror("Parent  : [sem_close] for hunterCanContinue Failed\n");
					return  3;
			 }
			 if( sem_unlink(hunterCanContinueName) < 0 ){
					printf("Parent  : [sem_unlink] for hunterCanContinueName Failed\n");
					return 6;
			 }

			 if (sem_close(thiefCanContinue) != 0){
					perror("Parent  : [sem_close] for thiefCanContinue Failed\n");
					return  3;
				}
				if( sem_unlink(thiefCanContinueName) < 0 ){
					printf("Parent  : [sem_unlink] for thiefCanContinueName Failed\n");
					return 6;
				}

				if (sem_close(playCountMutex) != 0){
						perror("Parent  : [sem_close] for playCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(playCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for playCountMutexName Failed\n");
						return 6;
				}

				if (sem_close(fightCountMutex) != 0){
						perror("Parent  : [sem_close] for fightCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(fightCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for fightCountMutexName Failed\n");
						return 6;
				}

				if (sem_close(thiefCountMutex) != 0){
						perror("Parent  : [sem_close] for thiefCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(thiefCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for thiefCountMutexName Failed\n");
						return 6;
				}

				if (sem_close(hunterCountMutex) != 0){
						perror("Parent  : [sem_close] for hunterCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(hunterCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for hunterCountMutexName Failed\n");
						return 6;
				}

				if (sem_close(thiefCanContinueCountMutex) != 0){
						perror("Parent  : [sem_close] for thiefCanContinueCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(thiefCanContinueCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for thiefCanContinueCountMutexName Failed\n");
						return 6;
				}

				if (sem_close(hunterCanContinueCountMutex) != 0){
						perror("Parent  : [sem_close] for hunterCanContinueCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(hunterCanContinueCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for hunterCanContinueCountMutexName Failed\n");
						return 6;
				}
				if (sem_close(smaugJewelCounterMutex) != 0){
						perror("Parent  : [sem_close] for smaugJewelCounterMutex Failed\n");
						return  3;
				}
				if( sem_unlink(smaugJewelCounterMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for smaugJewelCounterMutexName Failed\n"); //todo: change the error messages on all of these
						return 6;
				}
				if (sem_close(defeatedHuntersCountMutex) != 0){
						perror("Parent  : [sem_close] for defeatedHuntersCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(defeatedHuntersCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for defeatedHuntersCountMutexName Failed\n"); //todo: change the error messages on all of these
						return 6;
				}
				if (sem_close(defeatedThievesCountMutex) != 0){
						perror("Parent  : [sem_close] for defeatedThievesCountMutex Failed\n");
						return  3;
				}
				if( sem_unlink(defeatedThievesCountMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for defeatedThievesCountMutexName Failed\n"); //todo: change the error messages on all of these
						return 6;
				}
				if (sem_close(smaugActiveMutex) != 0){
						perror("Parent  : [sem_close] for smaugActiveMutex Failed\n");
						return  3;
				}
				if( sem_unlink(smaugActiveMutexName) < 0 ){
						printf("Parent  : [sem_unlink] for smaugActiveMutexName Failed\n"); //todo: change the error messages on all of these
						return 6;
				}

			//TODO: is it just unlink? why not close?

				if( sem_unlink(smaugJewelsCountName) < 0 ){
					printf("Parent  : [sem_unlink] for smaugJewelsCountName Failed\n");
					return 6;
				}

				if( sem_unlink(playCountName) < 0 ){
					printf("Parent  : [sem_unlink] for playCountName Failed\n");
					return 6;
				}

				if( sem_unlink(fightCountName) < 0 ){
					printf("Parent  : [sem_unlink] for fightCountName Failed\n");
					return 6;
				}

				if( sem_unlink(thiefCountName) < 0 ){
					printf("Parent  : [sem_unlink] for thiefCountName Failed\n");
					return 6;
				}

				if( sem_unlink(hunterCountName) < 0 ){
					printf("Parent  : [sem_unlink] for hunterCountName Failed\n");
					return 6;
				}

				if( sem_unlink(thiefCanContinueCountName) < 0 ){
					printf("Parent  : [sem_unlink] for thiefCanContinueCountName Failed\n");
					return 6;
				}

				if( sem_unlink(hunterCanContinueCountName) < 0 ){
					printf("Parent  : [sem_unlink] for hunterCanContinueCountName Failed\n");
					return 6;
				}

				if( sem_unlink(defeatedHuntersCountName) < 0 ){
					printf("Parent  : [sem_unlink] for defeatedHuntersCountName Failed\n");
					return 6;
				}

				if( sem_unlink(defeatedThievesCountName) < 0 ){
					printf("Parent  : [sem_unlink] for defeatedThievesCountName Failed\n");
					return 6;
				}

				if( sem_unlink(smaugActiveName) < 0 ){
					printf("Parent  : [sem_unlink] for smaugActiveName Failed\n");
					return 6;
				}

			exit(0);
	}/* Main process */
	else {
		pthread_t generateThievesPID;
		pthread_t generateTreasureHuntersPID;
		// create thief and hunter processes
		pthread_create(&generateThievesPID, NULL, generateThieves, NULL);
		//pthread_create(&generateTreasureHuntersPID, NULL, generateTreasureHunters, NULL);
		// join all threads
		pthread_join(&generateThievesPID,NULL);
		//pthread_join(&generateTreasureHuntersPID,NULL);

	}

	return 0;

}

void* generateThieves(){
	/*
	Description: This function generates the thief processes and 
	cleans up all the processes that are blocked when the smaug 
	process termintes. A thief checks whether its win prob is 
	at least the same as the winProb that is obtained from the user
	to secure a victory from smaug. If a thief wins, the thief 
	gets 6 jewels from Smaug's chest; otherwise, a thief loses 20
	jewels. 

	*/
	int currentIndex=0; 
	const int NUMTHIEVES=100, MAXTRAVELLINGTIME=10;

	long thievesPIDs[NUMTHIEVES];

	// shared memo vars
	int iSmaugActive;
	int smaugActiveFD;
	void *smaugActivePtr;
	void *smaugActive;

	// open the shared memory object
	smaugActiveFD=shm_open(smaugActiveName, O_CREAT | O_RDONLY, 0666);
	// pointer to shared memory
	smaugActivePtr=mmap(0, size, PROT_WRITE, MAP_SHARED, smaugActiveFD, 0);


	// shared sems vars
	const char* serviceIndexNameSem="/serviceIndexSem";
	sem_t *serviceIndexSem;	

	smaugActiveMutex = sem_open(smaugActiveMutexName,  O_CREAT, 0600, 1);
	if(smaugActiveMutex == SEM_FAILED){
			perror("Parent  : [sem_open] Failed\n");
			return 1;
	}


	// shared var between thief and generator
	const char* serviceIndexNameVar="/serviceIndexSem";
	int initValue=0;
	int serviceIndexFD;
	void* serviceIndexPtr;
	void* serviceIndex;	

	// open the memory loc
	serviceIndexFD=shm_open(serviceIndexNameVar, O_CREAT | O_RDRW, 0666);
	// pt to memory loc
	serviceIndexPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, serviceIndexFD, 0);

	// initialize the shared var between thief and generator
	memcpy(serviceIndex,serviceIndexPtr,size);
	// convert to integer
	int iserviceIndex= *((int*)serviceIndex);
	// update the number of thieves
	iserviceIndex= initValue;
	// convert to binary
	serviceIndex=(void*)iserviceIndex;
	// copy back to memory
	memcpy(serviceIndexPtr, serviceIndex, sizeof(serviceIndex));

	/*
    	SECTION: Generate all thief processes.  
    */
	while(1) {
		// checking if smaug is active:
		sem_wait(smaugActiveMutex);
		// copy from memory
		memcpy(smaugActive,smaugActivePtr,size);
		// convert to int 
		iSmaugActive=*((int*)smaugActive);
		if (iSmaugActive==0){
			sem_post(smaugActiveMutex);
			break;
		} 
		sem_post(smaugActiveMutex);

		// create new process 
		int id=fork();
		/* 		THIEF CODE: 	*/
		if(id == 0){ 
			/* open all semaphores */
			serviceIndexSem=sem_open(serviceIndexNameSem, O_CREAT, 0600, 0);

			play = sem_open(thiefPlay, O_CREAT, 0600, 0);
			if(play == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			thiefLeaving = sem_open(thiefDone, O_CREAT, 0600, 0);
			if(thiefLeaving == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			thiefCanContinue = sem_open(thiefCanContinueName, O_CREAT, 0600, 1);//NOTE: initialize to 0 or 1?
			if(thiefCanContinue == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			playCountMutex = sem_open(playCountMutexName,  O_CREAT, 0600, 1);
			if(playCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			thiefCountMutex = sem_open(thiefCountMutexName,  O_CREAT, 0600, 1);
			if(thiefCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			thiefCanContinueCountMutex = sem_open(thiefCanContinueCountMutexName,  O_CREAT, 0600, 1);
			if(thiefCanContinueCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			smaugJewelCounterMutex = sem_open(smaugJewelCounterMutexName,  O_CREAT, 0600, 1);
			if(smaugJewelCounterMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			defeatedThievesCountMutex = sem_open(defeatedThievesCountMutexName,  O_CREAT, 0600, 1);
			if(defeatedThievesCountMutex == SEM_FAILED){
					perror("Parent  : [sem_open] Failed\n");
					return 1;
			}

			/* 	opening shared memory */
			int size=4; // size in bytes
			// num of thieves
			int inumThieves;
			void *numThieves, *jewelCount;
			// shared memory file descriptor
			int jewelsCountFD; 
			int thiefCountFD;
			int defeatedCountFD; 
			// pointers to shared memory
			void* jewelsCountPtr;
			void* thiefCountPtr; 
			void* defeatedCountPtr;
			// create the shared memory object
			jewelsCountFD=shm_open(smaugJewelsCountName, O_CREAT | O_RDRW, 0666);
			thiefCountFD=shm_open(thiefCountName, O_CREAT | O_RDRW, 0666);
			defeatedCountFD=shm_open(defeatedThievesCountName, O_CREAT | O_RDRW, 0666);
			// pointer to shared object 
			jewelsCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, jewelsCountFD, 0);
			thiefCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, thiefCountFD, 0);
			defeatedCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, defeatedCountFD, 0);
			
			// write to thief memory -- add new thief:
			sem_wait(playCountMutex);
			// copy from memory
			memcpy(numThieves,thiefCountPtr,size);
			// convert to integer
			inumThieves= *((int*)numThieves);
			// update num of thieves
			inumThieves++;
			// convert to binary
			numThieves=(void*)inumThieves;
			// copy back to memory
			memcpy(thiefCountPtr, numThieves, sizeof(numThieves));
			sem_post(playCountMutex);


			/* STATE: Travelling */
			

			sem_wait(thiefCanContinueCountMutex);
			printf("Thief %li travelling to the valley.\n", getpid());
			fflush(stdout);

			// i think we need to sleep to simulate travelling to the valley
			// seeding
			int k=rand()%MAXTRAVELLINGTIME;
			usleep(k*ONESECOND);


			/* STATE: Wandering */


			printf("Thief %li wandering in the valley.\n", getpid());
			fflush(stdout);
			
			sem_post(thiefCanContinueCountMutex);

			sem_wait(play); // wait to play with Smaug

			
			/*STATE: Playing */
			

			printf("Thief %li playing with Smaug.\n", getpid());
			fflush(stdout);

			int thiefWinProb=1+rand()%100;
			if(thiefWinProb>=winProb){
				
				int ijewelCount;


				/* WIN */


				// change smaug's jewel count
				printf("Thief %li won and receives treasure.\n", getpid());
				fflush(stdout);

				/* write to jewel memory */
				sem_wait(smaugJewelCounterMutex);
				// copy from memory
				memcpy(jewelCount,jewelsCountPtr,size);
				// convert to integer
				ijewelCount= *((int*)jewelCount);
				// update
				ijewelCount-=8;
				// convert to bin
				jewelCount=(void*)ijewelCount;
				// copy back to memory
				memcpy(jewelsCountPtr, jewelCount, sizeof(jewelCount));
				sem_post(smaugJewelCounterMutex);


			} 
			else{
				
				int idefeadtedCount;
				void* defeadtedCount;

				/*		 LOSE		*/


				// change Smaug's jewel count
				
				printf("Thief %li has been defeated and pays the price.\n", getpid());
				
				/* write to memory */
				
				sem_wait(smaugJewelCounterMutex);
				// copy from memory
				memcpy(jewelCount,jewelsCountPtr,size);
				// convert to integer
				int ijewelCount= *((int*)jewelCount);
				// update
				ijewelCount+=20;
				// convert to bin
				jewelCount=(void*)ijewelCount;
				// copy back to memory
				memcpy(jewelsCountPtr, jewelCount, sizeof(jewelCount));
				sem_post(smaugJewelCounterMutex);

				/* write to defeated memory */
				
				sem_wait(defeatedThievesCountMutex);
				// copy from memory
				memcpy(defeadtedCount, defeatedCountPtr, size);
				// convert to integer
				int idefeatedCount=*((int*)defeadtedCount);
				// update
				idefeatedCount++;
				// convert to bin
				defeadtedCount=(void*)idefeatedCount;
				// copy back to memory
				memcpy(defeatedCountPtr, defeadtedCount, sizeof(defeadtedCount));
				sem_post(defeatedThievesCountMutex);
			}

			// 
			int iserviceIndex;
			int serviceIndexFD;
			void* serviceIndexPtr;
			void* serviceIndex;

			// open the memory loc
			serviceIndexFD=shm_open(serviceIndexNameSem, O_CREAT | O_RDRW, 0666);
			// pt to memory loc
			serviceIndexPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, serviceIndexFD, 0);

			/*		STATE: Termination		*/
			

			// write to thief count memory 
			sem_wait(playCountMutex);
			// copy from memory
			memcpy(numThieves,thiefCountPtr,size);
			// convert to integer
			inumThieves= *((int*)numThieves);
			// update the number of thieves
			inumThieves--;
			// convert to binary
			numThieves=(void*)inumThieves;
			// copy back to memory
			memcpy(thiefCountPtr, numThieves, sizeof(numThieves));
			sem_post(playCountMutex);

			// write to the serviced index memory
			sem_wait(serviceIndexSem);
			// copy from memory
			memcpy(serviceIndex,serviceIndexPtr,size);
			// convert to integer
			iserviceIndex= *((int*)serviceIndex);
			// update the number of thieves
			iserviceIndex= (iserviceIndex+1)%NUMTHIEVES;
			// convert to binary
			serviceIndex=(void*)iserviceIndex;
			// copy back to memory
			memcpy(serviceIndexPtr, serviceIndex, sizeof(serviceIndex));
			sem_post(serviceIndexSem);

			// signal smaug that i am done 
			sem_post(thiefLeaving);
			exit(0);

		}
		else {


			/* 	GENERATOR'S CODE:	 */

			int smaugStatus;
			currentIndex++;
			// wait for the process at head to finish before overwritting
			while (1){
				sem_wait(serviceIndexSem);

				if (currentIndex!=serviceIndex){
					sem_post(serviceIndexSem);
					break;
				}
				sem_post(serviceIndexSem);
			}
			// save the thief's id
			thievesPIDs[currentIndex%NUMTHIEVES]=id;
			// wait until before generating next thief
			// reseed
			int k=rand()%maximumThiefInterval;
			usleep(k*ONESECOND);

			sem_wait(smaugActiveMutex);
			// copy from memory
			memcpy(smaugActive,smaugActivePtr,size);
			// convert to int 
			iSmaugActive=*((int*)smaugActive);
			// check if smaug is active
			if (iSmaugActive==0){
				smaugStatus=0;
			} 
			sem_post(smaugActiveMutex);

			// checking if smaug is dead
			if (smaugStatus==0){
				/*
		    	SECTION: wait until have terminated 
		    	*/
		   		currentIndex = 0;
		   		int* thiefCountFD=shm_open(thiefCountName, O_CREAT | O_RDRW, 0666);
				void* thiefCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, thiefCountFD, 0);
				int inumThieves; 
				void* numThieves;
				// cpy num thieves in memory
				memcpy(numThieves,thiefCountPtr,size);
				// convert to integer
				inumThieves= *((int*)numThieves);
			    
			    // here we must kill all the waited processes using the kill sys call
			    int index=0;
			    while(1){
			    	if(index >= inumThieves){
			    		break
			    	}
			    	if (thievesPIDs[index] != 0) {
			    		kill(thievesPIDs[index], SIGKILL);
			    	}
			    	index++;
			    }
				
				// close + unlink semaphores
		    	sem_close(serviceIndexSem);
		    	sem_unlink(serviceIndexNameSem);
				// terminate thread
		   		 pthread_exit(NULL);
			}	
	}	


}
/*
void* generateTreasureHunters(){
	/*
	Description: 
	
	 generate H treasure hunters
}*/

int SmaugDeath(){

	/*
	Description: 

	*/
	int numHuntersLost = 0;
	void* nHL;
	int numThievesLost = 0;
	void* nTL;
	int numJewels = 0;
	void* nJ;
	int deathImmenent = 0;
	void* dI;

	sem_wait(defeatedHuntersCountMutex);
	memcpy(nHL,defeatedHuntersPTR, size);
	numHuntersLost = *((int*)nHL);
	sem_post(defeatedHuntersCountMutex);

	sem_wait(defeatedThievesCountMutex);
	memcpy(nTL,defeatedThievesPTR, size);
	numThievesLost = *((int*)nTL);
	sem_post(defeatedThievesCountMutex);

	sem_wait(smaugJewelCounterMutexName);
	memcpy(nJ, smaugJewels, size);
	numJewels =  *((int*)nJ);
	sem_post(smaugJewelCounterMutexName);

	if((numHuntersLost >=4) || (numThievesLost>=3)){deathImmenent=1;}
	if((numJewels <= 0) || (numJewels >= 80)){deathImmenent=1;}

	if(deathImmenent){
		sem_wait(smaugActive);
		dI = (void*)deathImmenent;
		memcpy(smaugActiveName, dI, sizeof(dI)); //NOTE: idk if correct
		sem_post(smaugActive);
	}
	return deathImmenent;
}