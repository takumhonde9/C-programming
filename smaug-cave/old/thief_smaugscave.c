#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>

int ONESECOND=1000000;

int main() {
	smaug_id=fork();
	/* Smaug process */
	if (smaug_id=0){

		exit(0);
	}
	else {
		

		/* Main process */


		pthread_t generateThievesPID;
		pthread_t generateTreasureHuntersPID;
		// create thief and hunter processes
		pthread_create(&generateThievesPID, NULL, generateThieves, NULL);
		pthread_create(&generateTreasureHuntersPID, NULL, generateTreasureHunters, NULL);
		// join all threads
		pthread_join(&generateThievesPID,NULL);
		pthread_join(&generateTreasureHuntersPID,NULL);

	}
	return 0;

}

void* generateThieves(){
	int currentIndex=0; 
	const int NUMTHIEVES=100;
	long thiefPIDs[NUMTHIEVES];

	// shared memo vars
	int iSmaugActive;
	int smaugActiveFD;
	void *smaugActivePtr;
	void *smaugActive;

	// open the shared memory object
	smaugActiveFD=shm_open(name, O_CREAT | O_RDONLY, 0666);
	// pointer to shared memory
	smaugActivePtr=mmap(0, size, PROT_WRITE, MAP_SHARED, smaugActiveFD, 0);


	// shared sems vars
	const char* serviceIndexNameSem="/serviceIndexSem";
	sem_t *serviceIndexSem;	

	// shared var between thief and generator
	const char* serviceIndexNameVar="/serviceIndexSem";
	int initValue=0;
	int serviceIndexFD;
	void* serviceIndexPtr;
	void* serviceIndex;	

	// open the memory loc
	serviceFD=shm_open(name, O_CREAT | O_RDRW, 0666);
	// pt to memory loc
	serviceIndexPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, serviceIndexFD, 0);

	// initialize the shared var between thief and generator
	memcpy(serviceIndex,serviceIndexPtr,size);
	// convert to integer
	iserviceIndex= *((int*)serviceIndex);
	// update the number of thieves
	iserviceIndex= initValue;
	// convert to binary
	serviceIndex=(void*)iserviceIndex;
	// copy back to memory
	memcpy(serviceIndexPtr, serviceIndex, sizeof(serviceIndex));

	/*
    	SECTION: Generate all thief processes.  
    */
	while(true) {

		sem_wait();
		// copy from memory
		memcpy(smaugActive,smaugActivePtr,size);
		// convert to int 
		iSmaugActive=*((int*)smaugActive);
		// check if smaug is active
		if (iSmaugActive==0){
			sem_post();
			break;
		} 
		sem_post();

		// create new process 
		id=fork();
		/* 		THIEF CODE: 	*/
		if(id == 0){ 
			/* open all semaphores */
			serviceIndexSem=sem_open(serviceIndexName, O_CREAT, 0600, 0);


			/* 	opening shared memory */
			int size=4; // size in bytes
			// num of thieves
			int inumThieves;
			void *numThieves, *jewelCount;
			// shared memory file descriptor
			int jewelCountFD; 
			int thiefCountFD;
			int defeatedCountFD; 
			// pointers to shared memory
			void* jewelsPtr;
			void* thiefCountPtr; 
			void* defeatedCountPtr;
			// create the shared memory object
			jewelsCountFD=shm_open(name, O_CREAT | O_RDRW, 0666);
			thiefCountFD=shm_open(name, O_CREAT | O_RDRW, 0666);
			defeatedCountFD=shm_open(name, O_CREAT | O_RDRW, 0666);
			// pointer to shared object 
			jewelsCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, jewelsFD, 0);
			thiefCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, thiefCountFD, 0);
			defeatedCountPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, defeatedCountFD, 0);
			
			// write to thief memory -- add new thief:
			sem_wait();
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
			sem_post();


			/* STATE: Travelling */
			

			sem_wait(&theifCanContinue);
			printf("Thief %li travelling to the valley.\n", getpid());
			fflush(stdout);

			// i think we need to sleep to simulate travelling to the valley
			int k=rand%MAXTRAVELLINGTIME;
			usleep(k*ONESECOND);


			/* STATE: Wandering */


			printf("Thief %li wandering in the valley.\n", getpid());
			fflush(stdout);
			
			sem_post(thiefCanContinue);

			sem_wait(play); // wait to play with Smaug

			
			/*STATE: Playing */
			

			printf("Thief %li playing with Smaug.\n", getpid());
			fflush(stdout);

			int thiefWinProb=1+rand%100
			if(thiefWinProb>=winProb){
				
				int ijewelCount;

				/* WIN */


				// change smaug's jewel count
				printf("Thief %li won and receives treasure.\n", getpid());
				fflush(stdout);

				/* write to jewel memory */
				sem_wait();
				// copy from memory
				memcpy(jewelCount,jewelCountPtr,size);
				// convert to integer
				int ijewelCount= *((int*)jewelCount);
				// update
				ijewelCount-=8;
				// convert to bin
				jewelCount=(void*)ijewelCount;
				// copy back to memory
				memcpy(jewelCountPtr, jewelCount, sizeof(jewelCount));
				sem_post();


			} 
			else{
				
				int idefeadtedCount;
				void* defeadtedCount;

				/*		 LOSE		*/


				// change Smaug's jewel count
				
				printf("Thief %li has been defeated and pays the price.\n", getpid());
				fflush(stdout)
				
				/* write to memory */
				
				sem_wait();
				// copy from memory
				memcpy(jewelCount,jewelCountPtr,size);
				// convert to integer
				int ijewelCount= *((int*)jewelCount);
				// update
				ijewelCount+=20;
				// convert to bin
				jewelCount=(void*)ijewelCount;
				// copy back to memory
				memcpy(jewelCountPtr, jewelCount, sizeof(jewelCount));
				sem_post();

				/* write to defeated memory */
				
				sem_wait();
				// copy from memory
				memcpy(defeadtedCount, defeatedCountPtr, size);
				// convert to integer
				idefeatedCount=*((int*)defeadtedCount);
				// update
				idefeatedCount++;
				// convert to bin
				defeadtedCount=(void*)idefeatedCount;
				// copy back to memory
				memcpy(defeatedCountPtr, defeadtedCount, sizeof(defeadtedCount));
				sem_post();
			}

			// 
			int iserviceIndex;
			int serviceIndexFD;
			void* serviceIndexPtr;
			void* serviceIndex;

			// open the memory loc
			serviceFD=shm_open(name, O_CREAT | O_RDRW, 0666);
			// pt to memory loc
			serviceIndexPtr=mmap(0, size, PROT_WRITE, MAP_SHARED, serviceIndexFD, 0);

			/*		STATE: Termination		*/
			

			// write to thief count memory 
			sem_wait();
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
			sem_post();

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
			sem_post(&thiefDone);
			exit(0);

		}
		else {


			/* 	GENERATOR'S CODE:	 */


			currentIndex++;
			// wait for the process at head to finish before overwritting
			while (true){
				sem_wait(serviceIndexSem)

				if (currentIndex!=serviceIndex){
					sem_post(serviceIndexSem)
					break;
				}
				sem_post(serviceIndexSem);
			}
			// save the thief's id
			thievesPIDs[currentIndex%NUMTHIEVES]=id;
			// wait until before generating next thief
			int k=rand%maxThiefInterval;
			usleep(k*ONESECOND);
		}
	}
	/*
    	SECTION: Terminate all processes generated by the thread 
    */
    index = 0;
    while (1){
        int pid = waitpid(childrenPIDs[index], NULL, 0); 
        if(pid>0) {
            index++;
        if (index >= numThieves-1) {
        	//
            break;
        }
    }
    // unlink semaphore
    sem_close(serviceIndexSem);
    sem_unlink(serviceIndexNameSem);

    // terminate thread
    pthread_exit(NULL)

}	
/*
void* generateTreasureHunters(){ // wins 10 lose 5
	// generate H treasure hunters 

}*/