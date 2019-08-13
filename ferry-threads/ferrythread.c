/*

Title: Ferry Simulation

Description: This code simulates a ferry system that carries cars and/or trucks
	from a start port to a destination port. A captain, start port (which generates
	cars and trucks), cars and trucks are represented as threads, so that each of
	them can operate independently. The ferry, operated by the captain, only takes
	5 trips and can carry a limited number of vehicles and the start port can generate
	an infinite amount of vehicles.

Names: Gurkiran Brar (301274688), Sheel Soneji (301295318), Takudzwa Mhonde (301270018)


Last edited date: 3/4/19

*/

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h> // usleep()
#include <sys/time.h> // gettimeofday()
#include <sys/types.h> // (unsigned int)pthread_self()
#include <stdlib.h> //
#include <stdbool.h> // bool

void* createVehicles();
void* car();
void* truck();
void* captain();
int timeChange( const struct timeval startTime );
bool fullyLoaded();

//limitations on creation of vehicles:
const int MAX_CARS = 30;
const int MAX_TRUCKS = 10;
const int MAX_TRIPS = 5;
const int ONE_SECOND=1000000;
int num_trips;
int num_cars_ferry; //cars in ferry in total
int num_trucks_ferry; //trucks in ferry in total
int num_created_cars;
int num_created_trucks;
pthread_t cars_created[1000]; //NOTE: needs to be bigger
pthread_t trucks_created[1000]; //NOTE: needs to be bigger


//mutexes:
pthread_mutex_t car_queue_mutex;
pthread_mutex_t truck_queue_mutex;
pthread_mutex_t car_loaded_mutex;
pthread_mutex_t truck_loaded_mutex;
pthread_mutex_t car_sailing_mutex;
pthread_mutex_t truck_sailing_mutex;
pthread_mutex_t car_unloaded_mutex;
pthread_mutex_t truck_unloaded_mutex;
pthread_mutex_t printf_mutex;
pthread_mutex_t trip_mutex;
//pthread_mutex_t start_queue_index_car_mutex;
//pthread_mutex_t start_queue_index_truck_mutex;

//semaphores:
sem_t car_queue_sem;
sem_t truck_queue_sem;
sem_t car_loaded_sem;
sem_t truck_loaded_sem;
sem_t car_sailing_sem;
sem_t truck_sailing_sem;
sem_t car_unloaded_sem;
sem_t truck_unloaded_sem;
sem_t car_terminal_sem;
sem_t truck_terminal_sem;

//counter variables:
int num_car_queue;
int num_truck_queue;
int num_car_loaded;
int num_truck_loaded;
int num_car_sailing;
int num_truck_sailing;
int num_car_unloaded;
int num_truck_unloaded;

int K_max; //max time
int seed;
int probability; // probability a vehicle will be a truck




int main(){
	//initialize variables:
	num_trips = 0;
	num_car_queue = 0;
	num_truck_queue = 0;
	num_car_loaded = 0;
	num_truck_loaded = 0;
	num_car_sailing = 0;
	num_truck_sailing = 0;
	num_car_unloaded = 0;
	num_truck_unloaded = 0;
	K_max = 0;
	seed = 0;
	probability = 0;
	num_cars_ferry = 0;
	num_trucks_ferry = 0;
	num_created_cars = 0;
	num_created_trucks = 0;

	pthread_mutex_init(&car_queue_mutex, NULL);
	pthread_mutex_init(&truck_queue_mutex, NULL);
	pthread_mutex_init(&car_loaded_mutex, NULL);
	pthread_mutex_init(&truck_loaded_mutex, NULL);
	pthread_mutex_init(&car_sailing_mutex, NULL);
	pthread_mutex_init(&truck_sailing_mutex, NULL);
	pthread_mutex_init(&car_unloaded_mutex, NULL);
	pthread_mutex_init(&truck_unloaded_mutex, NULL);
	pthread_mutex_init(&printf_mutex, NULL);
	pthread_mutex_init(&trip_mutex, NULL);

	sem_init(&car_queue_sem , 0, 0);
	sem_init(&truck_queue_sem , 0, 0);
	sem_init(&car_loaded_sem , 0, 0);
	sem_init(&truck_loaded_sem , 0, 0);
	sem_init(&car_sailing_sem , 0, 0);
	sem_init(&truck_sailing_sem , 0, 0);
	sem_init(&car_unloaded_sem , 0, 0);
	sem_init(&truck_unloaded_sem , 0, 0);
	sem_init(&car_terminal_sem, 0, 0);
	sem_init(&truck_terminal_sem, 0, 0);

	//values:
	printf("Please enter integer values for the following variables\n"); //TODO: deal with incorrect data - aka too large number
	printf("Enter the percent probability that the next vehicle is a truck:\n");
	scanf("%i", &probability);
	printf("Enter the maximum length of the interval between vehicle:\ntime interval should be >1000:\n");
	scanf("%i", &K_max); // idk if we need to do anything with this...
	printf("Enter the seed for random number generation:\n");
	scanf("%i", &seed);
	//create threads:
	pthread_t createVehicle_pid;
	pthread_t captain_pid;

	pthread_create( &createVehicle_pid , NULL, createVehicles, NULL);
	pthread_create( &captain_pid , NULL, captain, NULL);
	//create ferry thread
	pthread_join(createVehicle_pid, NULL); //TODO: Join everything
	pthread_join(captain_pid, NULL); //TODO: Join everything

	int num_trucks_left = num_created_trucks - num_trucks_ferry;
	int num_cars_left = num_created_cars - num_cars_ferry;

	for(int i = 0; i < num_trucks_left; i++){ pthread_join(trucks_created[num_created_trucks+i], NULL);}
	for(int i = 0; i < num_cars_left; i++){ pthread_join(cars_created[num_created_cars+i], NULL);}

	sem_destroy(&car_queue_sem);
	sem_destroy(&truck_queue_sem);
	sem_destroy(&car_loaded_sem);
	sem_destroy(&truck_loaded_sem);
	sem_destroy(&car_sailing_sem);
	sem_destroy(&truck_sailing_sem);
	sem_destroy(&car_unloaded_sem);
	sem_destroy(&truck_unloaded_sem);
	sem_destroy(&car_terminal_sem);
	sem_destroy(&truck_terminal_sem);

	pthread_mutex_destroy(&car_queue_mutex);
	pthread_mutex_destroy(&truck_queue_mutex);
	pthread_mutex_destroy(&car_loaded_mutex);
	pthread_mutex_destroy(&truck_loaded_mutex);
	pthread_mutex_destroy(&car_sailing_mutex);
	pthread_mutex_destroy(&truck_sailing_mutex);
	pthread_mutex_destroy(&car_unloaded_mutex);
	pthread_mutex_destroy(&truck_unloaded_mutex);
	pthread_mutex_destroy(&printf_mutex);
	pthread_mutex_destroy(&trip_mutex);

	return 0;
}

int timeChange( const struct timeval startTime )
{
   struct timeval nowTime;
    long int elapsed;
    int elapsedTime;

    gettimeofday(&nowTime, NULL);
    elapsed = (nowTime.tv_sec - startTime.tv_sec) * 1000000
          + (nowTime.tv_usec - startTime.tv_usec);
    elapsedTime = elapsed / 1000;
    return elapsedTime;
}

void* createVehicles(){
	//create car and truck threads here
	printf("CREATEVEHICLE:	Vehicle creation thread has been started\n");
	srand(seed);
	int next_arrival_time=0;
	struct timeval startTime;
	gettimeofday(&startTime, NULL);

	while(1){ //NOTE: we are assuming the stream of cars never stops
		pthread_mutex_lock(&trip_mutex);
		if(num_trips>=MAX_TRIPS){
			pthread_mutex_unlock(&trip_mutex);
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&trip_mutex);

		int time_dif=timeChange(startTime);

		if(time_dif > next_arrival_time){//create vehicle here
			pthread_mutex_lock(&printf_mutex);
			printf("CREATEVEHICLE:	Elapsed time %i msec\n", time_dif);
			fflush(stdout);
			pthread_mutex_unlock(&printf_mutex);
			int vehicle_type = rand()%100;

			if((vehicle_type <= probability) && (num_created_trucks < 1000)){
				 //Create truck thread
				 pthread_create(&trucks_created[num_created_trucks], NULL, truck, NULL);
				 num_created_trucks++;
				 pthread_mutex_lock(&printf_mutex);
				 printf("CREATEVEHICLE:	Created a truck thread\n");
				 fflush(stdout);
				 pthread_mutex_unlock(&printf_mutex);
			}
			else{
				//Create car thread
				if(num_created_cars < 1000){
					pthread_create(&cars_created[num_created_cars], NULL, car, NULL);
					num_created_cars++;
					pthread_mutex_lock(&printf_mutex);
					printf("CREATEVEHICLE:	Created a car thread\n");
					pthread_mutex_unlock(&printf_mutex);
				}
			}
		}
		next_arrival_time = 1000+rand()%K_max;
		printf("CREATEVEHICLE:	Next arrival time %i msec\n", next_arrival_time); // NEXT ARRIVAL TIME?
		usleep(ONE_SECOND);
	}

}


void* car(){
		pthread_mutex_lock(&printf_mutex);
		printf("CAR:	Car with threadID %i queued\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);
		//get in waiting queue:
		pthread_mutex_lock(&car_queue_mutex);
		num_car_queue++;
		pthread_mutex_unlock(&car_queue_mutex);
		sem_wait(&car_queue_sem);
		//check if ferry closed:
		pthread_mutex_lock(&trip_mutex);
		if(num_trips == MAX_TRIPS) { pthread_exit(NULL);}
		pthread_mutex_unlock(&trip_mutex);

		pthread_mutex_lock(&printf_mutex);
		printf("CAR:	Car with threadID %i leaving the queue to load\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		//load ferry:
		pthread_mutex_lock(&car_loaded_mutex);
		num_car_loaded++;
		pthread_mutex_unlock(&car_loaded_mutex);
		pthread_mutex_lock(&car_queue_mutex);
		num_car_queue--;
		pthread_mutex_unlock(&car_queue_mutex);
		sem_post(&car_loaded_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("CAR:	Car with threadID %i is onboard the ferry\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		//onboard ferry:
		pthread_mutex_lock(&car_sailing_mutex);
		num_car_sailing++;
		pthread_mutex_unlock(&car_sailing_mutex);
		sem_wait(&car_sailing_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("CAR:	Car with threadID %i is now unloading\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		//unloading:
		pthread_mutex_lock(&car_unloaded_mutex);
		num_car_unloaded++;
		pthread_mutex_unlock(&car_unloaded_mutex);
		sem_post(&car_unloaded_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("CAR:	Car with threadID %i has unloaded\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		sem_wait(&car_terminal_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("CAR:	Car with threadID %i is about to exit\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		pthread_exit(NULL);
}


void* truck(){

		pthread_mutex_lock(&printf_mutex);
		printf("TRUCK:	Truck with threadID %i queued\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);
		//get in waiting queue:
		pthread_mutex_lock(&truck_queue_mutex);
		num_truck_queue++;
		pthread_mutex_unlock(&truck_queue_mutex);
		sem_wait(&truck_queue_sem);
		//check if ferry closed:
		pthread_mutex_lock(&trip_mutex);
		if(num_trips == MAX_TRIPS) { pthread_exit(NULL);}
		pthread_mutex_unlock(&trip_mutex);

		pthread_mutex_lock(&printf_mutex);
		printf("TRUCK:	Truck with threadID %i leaving the queue to load\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		//load ferry:
		pthread_mutex_lock(&truck_loaded_mutex);
		num_truck_loaded++;
		pthread_mutex_unlock(&truck_loaded_mutex);
		pthread_mutex_lock(&truck_queue_mutex);
		num_truck_queue--;
		pthread_mutex_unlock(&truck_queue_mutex);
		sem_post(&truck_loaded_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("TRUCK:	Truck with threadID %i is onboard the ferry\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

//onboard ferry:
		pthread_mutex_lock(&truck_sailing_mutex);
		num_truck_sailing++;
		pthread_mutex_unlock(&truck_sailing_mutex);
		sem_wait(&truck_sailing_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("TRUCK:	Truck with threadID %i is now unloading\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		//unloading:
		pthread_mutex_lock(&truck_unloaded_mutex);
		num_truck_unloaded++;
		pthread_mutex_unlock(&truck_unloaded_mutex);
		sem_post(&truck_unloaded_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("TRUCK:	Truck with threadID %i has unloaded\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		sem_wait(&truck_terminal_sem);

		pthread_mutex_lock(&printf_mutex);
		printf("TRUCK:	Truck with threadID %i is about to exit\n", (unsigned int)pthread_self());
		fflush(stdout);
		pthread_mutex_unlock(&printf_mutex);

		pthread_exit(NULL);
}

void* captain(){
    /*
    Description: Captain thread function. Initially, the captain waits until there are enough
    vehicles at the dock to start it's trips. The captain only does a maximum of 5 trips per
    day, taking vehicles from one dock to the other.

    NOTE: The captain can board a maximum of 6 cars, space-wise (where 1 truck takes up an
    equivalent of 2 car spaces). Also the ferry can only carry a maximum of two trucks.
    */
    pthread_mutex_lock(&printf_mutex);
    printf("%s\n", "CAPTAIN:        Captain thread has started");
    fflush(stdout);
    pthread_mutex_unlock(&printf_mutex);
    /*  SECTION 1: WAITING FOR SOME VEHICLES BEFORE I START MY DAY  */
	int temp_num_cars=0;
	int temp_num_trucks=0;
    while(1) {
        // lock mutexes to check num of cars and trucks
		pthread_mutex_lock(&car_queue_mutex);
		temp_num_cars=num_car_queue;
		pthread_mutex_unlock(&car_queue_mutex);

		pthread_mutex_lock(&truck_queue_mutex);
		temp_num_trucks=num_truck_queue;
		pthread_mutex_unlock(&truck_queue_mutex);

        pthread_mutex_lock(&printf_mutex);
        printf("CAPTAIN:        Captain process intial %d vehicles.\n", temp_num_cars+temp_num_trucks);
        fflush(stdout);
        pthread_mutex_unlock(&printf_mutex);

        // do I have enough vehicles to board ferry optimally?
        if (temp_num_trucks+temp_num_cars>=8){
            // STOP looping can now travel
            break;
		}
        // wait for one second before checking again
        usleep(ONE_SECOND);
    }
    /*  SECTION 2: STARTING MY DAY  */

    while(num_trips<MAX_TRIPS) {
        /*  DAY BEGINNING  */
        int num_car_recorded=0;
        int num_truck_recorded=0;
		int num_truck_onboard=0;
        // START: the captain's logic
        while(1){
            // Do I have some trucks that I can load?
            pthread_mutex_lock(&truck_queue_mutex);
            if(num_truck_queue>0 && num_truck_onboard<2){
                pthread_mutex_unlock(&truck_queue_mutex);


                /*  STATE: Loading truck */


                pthread_mutex_lock(&printf_mutex);
                printf("CAPTAIN:        Truck selected for loading.\n");
                fflush(stdout);
                pthread_mutex_unlock(&printf_mutex);
                // signal a truck to load itself
                sem_post(&truck_queue_sem);
                // wait for the truck to load itself
                sem_wait(&truck_loaded_sem);
                // Continue ...

                pthread_mutex_lock(&truck_loaded_mutex);
                num_truck_recorded=num_truck_loaded;
                pthread_mutex_unlock(&truck_loaded_mutex);
				num_truck_onboard++;
                pthread_mutex_lock(&printf_mutex);
                printf("CAPTAIN:        Captain knows truck is loaded.\n");
                fflush(stdout);
                pthread_mutex_unlock(&printf_mutex);

            } else { // loaded enough trucks, so I must load cars now
                pthread_mutex_unlock(&truck_queue_mutex);  // prevent deadlock


                /*  STATE: Loading car */


                pthread_mutex_lock(&car_queue_mutex);
                // are there cars at the dock
                if(num_car_queue>0){

                    pthread_mutex_unlock(&car_queue_mutex);
                    pthread_mutex_lock(&printf_mutex);
                    printf("CAPTAIN:        Car selected for loading.\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&printf_mutex);

                    sem_post(&car_queue_sem);
                    // wait for the car to load itself

                    sem_wait(&car_loaded_sem);

                    // Continue ...
                    pthread_mutex_lock(&car_loaded_mutex);
                    num_car_recorded=num_car_loaded;
                    pthread_mutex_unlock(&car_loaded_mutex);

                    pthread_mutex_lock(&printf_mutex);
                    printf("CAPTAIN:        Captain knows car is loaded.\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&printf_mutex);

                } else{ // no cars
                    pthread_mutex_unlock(&car_queue_mutex); // prevent deadlock
                }

            }
            // do I now have enough vehicles to go on trip?
            if(fullyLoaded()){
                pthread_mutex_lock(&printf_mutex);
                printf("CAPTAIN:        Ferry is full, starting to sail.\n");
                fflush(stdout);
                pthread_mutex_unlock(&printf_mutex);


                /* state: SAILING */


                usleep(2*ONE_SECOND);


                /* STATE: Arrival */
                pthread_mutex_lock(&printf_mutex);
                printf("CAPTAIN:        Ferry has reached the destination port.\n");
                fflush(stdout);
                pthread_mutex_unlock(&printf_mutex);

                // tell vehicles to unload -- missing code


                /* STATE:  Unloading*/


                while (1){ // TRUCKS unloading
                    if(num_truck_recorded==0) {
                        // No trucks on board
                        break;
                    }
                    // signal truck to to unboard
                    sem_post(&truck_sailing_sem);

                    // wait for the truck to unboard
                    sem_wait(&truck_unloaded_sem);

                    pthread_mutex_lock(&printf_mutex);
                    printf("CAPTAIN:        Captain knows a truck has unloaded from ferry.\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&printf_mutex);

                    // signal truck it can leave dock
                    sem_post(&truck_terminal_sem);

                    pthread_mutex_lock(&printf_mutex);
                    printf("CAPTAIN:        Captain sees a truck leaving the ferry terminal.\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&printf_mutex);

                    pthread_mutex_lock(&truck_unloaded_mutex);
                    if(num_truck_unloaded == num_truck_recorded) {
                        // Finished unloading trucks
                        pthread_mutex_unlock(&truck_unloaded_mutex);
                        break;
                    }
                    else {
                        pthread_mutex_unlock(&truck_unloaded_mutex); // prevent deadlock
                    }
                }

                while (1){ //CARS unloading
                    // signal truck to to unboard
                    sem_post(&car_sailing_sem);

                    // wait for the truck to unboard
                    sem_wait(&car_unloaded_sem);

                    pthread_mutex_lock(&printf_mutex);
                    printf("CAPTAIN:        Captain knows a truck has unloaded from ferry.\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&printf_mutex);

                    // signal truck it can leave dock
                    sem_post(&car_terminal_sem);

                    pthread_mutex_lock(&printf_mutex);
                    printf("CAPTAIN:        Captain sees a truck leaving the ferry terminal.\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&printf_mutex);

                    pthread_mutex_lock(&truck_unloaded_mutex);
                    if(num_truck_unloaded == num_truck_recorded) {
                        // Finished unloading trucks
                        pthread_mutex_unlock(&truck_unloaded_mutex);
                        break;
                    }
                    else {
                        pthread_mutex_unlock(&truck_unloaded_mutex); // prevent deadlock
                    }

                }
                // completed trip
                pthread_mutex_lock(&trip_mutex);
                num_trips++;
                pthread_mutex_unlock(&trip_mutex);
                break;
            }
            // keep loading and waiting
        }

        num_trucks_ferry+=num_truck_recorded;
        num_cars_ferry+=num_car_recorded;

        // RESET: loaded, sailing & unloaded counters

		pthread_mutex_lock(&car_loaded_mutex);
		num_car_loaded=0;
		pthread_mutex_unlock(&car_loaded_mutex);

		pthread_mutex_lock(&car_unloaded_mutex);
		num_car_unloaded=0;
		pthread_mutex_unlock(&car_unloaded_mutex);

		pthread_mutex_lock(&car_sailing_mutex);
		num_car_sailing=0;
		pthread_mutex_unlock(&car_sailing_mutex);

		pthread_mutex_lock(&truck_loaded_mutex);
		num_truck_loaded=0;
		pthread_mutex_unlock(&truck_loaded_mutex);

		pthread_mutex_lock(&truck_unloaded_mutex);
		num_truck_unloaded=0;
		pthread_mutex_unlock(&truck_unloaded_mutex);

		pthread_mutex_lock(&truck_sailing_mutex);
		num_truck_sailing=0;
		pthread_mutex_unlock(&truck_sailing_mutex);


    }
    pthread_exit(NULL);
}

// AUXILLARY FUNCTIONS
bool fullyLoaded(){
    /*
    Description: This function is used to check if the ferry has loaded trucks and
    satisfied the requirements mentioned: ferry has max space of 6 vehicles.
    */
    if ((num_car_loaded==6) || (num_car_loaded==2 && num_truck_loaded==2) || (num_car_loaded==4 && num_truck_loaded==1)){
        return true;
    }
    return false;
}
