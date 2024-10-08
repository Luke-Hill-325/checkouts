#include <iostream>
#include <ctime>
#include <string.h>

int randomRuns = 0;
int randomNat(int N){
	std::srand(std::time(NULL)+randomRuns);
	randomRuns++;
	return std::rand() % N; 
}
int randomPos(int N){
	return randomNat(N-1) + 1;
}
int randomInterval(int A, int B){
	return A + randomNat(B - A);
}
bool randomEvent(int N){
	return randomNat(N) == 0;
}

typedef int SimTime;
typedef int Duration;
typedef int ID;
struct Customer {
	ID custID;
	SimTime arrivalTime;
	Duration serviceTime;
};
class CustomerQueue {
	private:
		Customer* q = NULL;
		int capacity;
		int front;
		int back;
		int size;

		void grow() {
			Customer* temp = new Customer[capacity * 2];
			while(size){
				temp[capacity - size] = dequeue();
			}
			size = capacity;
			back = capacity;
			capacity *= 2;
			front = 0;
			delete[] q;
			q = temp;
		}
	public:
		CustomerQueue(){
			capacity = 1;
			size = 0;
			q = new Customer[capacity];
			front = 0;
			back = 0;
		}

		int getSize(){
			return size;
		}

		void enqueue(Customer c){
			if (size == capacity) {
				grow();
			}
			q[back] = c;
			back = (back + 1) % capacity;
			size++;
		}

		Customer dequeue(){
			if (size == 0) {
				printf("queue empty");
				Customer c;
				return c;
			}
			int f = front;
			front = (front + 1) % capacity;
			size--;
			return q[f];
		}

		int estimateWaitTime(){
			int waitTime = 0;
			for (int i = front; i != back; i = (i + 1) % capacity){
				waitTime += q[i].serviceTime;
			}
			return waitTime;
		}
};
struct ServiceStation {
	Customer served;
	Duration time2Serve;
	CustomerQueue queue;
	int timeBusy = 0;
	int EODlength;
	int EODtimeRemaining;
	int lineSize(){
		int size = queue.getSize();
		if(time2Serve){
			return size + 1;
		}
		return size;
	}
	bool service(){
		if(!time2Serve){
			if (queue.getSize() > 0){
				served = queue.dequeue();
				time2Serve = served.serviceTime;
			} else {
				return false;
			}
		}
		if(--time2Serve > 0){
			timeBusy++;
			return false;
		} else {
			if (queue.getSize()){
				served = queue.dequeue();
				time2Serve = served.serviceTime;
			}
			return true;
		}
	}
};
enum Strategy { roundRobin, randomQ, shortest };
int main(int argc, char* argv[]){
	Strategy strat;
	std::string infoOut = "Use args [roundrobin | random | shortest]";
	if(argc < 2){
		std::cout << infoOut << std::endl;
		return 0;
	}
	std::string stratArg = argv[1];
	if(!stratArg.compare("roundrobin")){
		strat = roundRobin;
	} else if(!stratArg.compare("random")){
		strat = randomQ;
	} else if(!stratArg.compare("shortest")){
		strat = shortest;
	}
	int NUM_STATIONS = 3;
	Duration SIMDURATION = std::stoi(argv[2]);
	int ARRIVAL_RATE = std::stoi(argv[3]);
	int SERVICERATE = std::stoi(argv[4]);
	ServiceStation stations[NUM_STATIONS];
	SimTime t = 0;
	int IDCounter = 1;
	int nextStation = 0;
	bool simOver = false;
	int highestStationLength = 0;
	long totalWaitTime = 0;
	int numServed = 0;
	do {
		if (randomEvent(ARRIVAL_RATE) && t < SIMDURATION) {
			simOver = false;
			Customer c;
			c.custID = IDCounter;
			IDCounter++;
			c.arrivalTime = t;
			c.serviceTime = randomPos(SERVICERATE);
			stations[nextStation].queue.enqueue(c);
			if (stations[nextStation].lineSize() > highestStationLength){
				highestStationLength++;
			}
			switch(strat){
				case roundRobin:
					nextStation = (nextStation + 1) % NUM_STATIONS;
					break;
				case randomQ:
					nextStation = randomNat(NUM_STATIONS);
					break;
				case shortest:
					{
						int shortestStation = 0;
						int shortestMeasure = stations[0].lineSize();
						for (int i = 1; i < NUM_STATIONS; i++){
							int measure = stations[i].lineSize();
							if (measure < shortestMeasure){
								shortestStation = i;
								shortestMeasure = measure;
							}
						}
						nextStation = shortestStation;
					}
					break;
			}
		}
		for (int i = 0; i < NUM_STATIONS; i++){
			if (stations[i].service()){
				totalWaitTime += t - stations[i].served.arrivalTime;	
				numServed++;
			}
			if (t == SIMDURATION){
				stations[i].EODlength = stations[i].lineSize();
				stations[i].EODtimeRemaining = stations[i].time2Serve + stations[i].queue.estimateWaitTime();
			}
		}
		t += 1;
		if (t > SIMDURATION){
			int remainingCustomers = 0;
			simOver = true;
			for (int i = 0; i < NUM_STATIONS; i++){
				if (stations[i].lineSize() > 0){
					remainingCustomers += stations[i].lineSize();
					simOver = false;
				}
			}
		}
	}while (!(simOver));
	std::cout << std::endl << "Strategy: " << argv[1] << std::endl << "number served: " << numServed << std::endl;
	printf("duration: ");
	std::cout << t << std::endl;
	printf("average wait time: ");
	std::cout << totalWaitTime / ((long)IDCounter - 1) << std::endl;
	for (int i = 0; i < NUM_STATIONS; i++) {
		std::cout << "station #" << i << std:: endl << "\tbusy ratio: " << (double)stations[i].timeBusy / (double)t << std::endl << "\tEnd Of Day Line Size: " << stations[i].EODlength << std::endl;
		std::cout << "\tEnd Of Day Wait time left: " << stations[i].EODtimeRemaining << std::endl;
	}
}
