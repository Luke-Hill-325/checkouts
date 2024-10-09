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
typedef long longDuration;
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

		Duration estimateWaitTime(){
			Duration waitTime = 0;
			for (int i = front; i != back; i = (i + 1) % capacity){
				waitTime += q[i].serviceTime;
			}
			return waitTime;
		}
};
enum Strategy { roundRobin, randomQ, shortest, oneQ};
struct ServiceStation {
	Customer served;
	Duration time2Serve;
	CustomerQueue queue;
	Duration timeBusy = 0;
	longDuration totalWaitTime = 0;
	int EODlength;
	Duration EODtimeRemaining;
	int numServed = 0;
	Duration maxWaitTime = 0;

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
		timeBusy++;
		if(--time2Serve > 0){
			return false;
		} else {
			if (queue.getSize()){
				served = queue.dequeue();
				time2Serve = served.serviceTime;
			}
			numServed++;
			return true;
		}
	}
};
int main(int argc, char* argv[]){
	Strategy strat;
	std::string infoOut = "Use args [roundrobin | random | shortest | onequeue] [int SIMDURATION] [int ARRIVAL_RATE] [int SERVICERATE]";
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
	} else if (!stratArg.compare("onequeue")){
		strat = oneQ;
	}

	int NUM_STATIONS = 5;
	Duration SIMDURATION = std::stoi(argv[2]);
	int ARRIVAL_RATE = std::stoi(argv[3]);
	int SERVICERATE = std::stoi(argv[4]);
	ServiceStation stations[NUM_STATIONS];
	SimTime t = 0;
	int IDCounter = 1;
	int nextStation = 0;
	bool simOver = false;
	int highestStationLength = 0;
	int numServed = 0;
	CustomerQueue arrivalQueue;
	longDuration totalWaitTime = 0;
	Duration maxWaitTime = 0;

	do {
		if (randomEvent(ARRIVAL_RATE) && t < SIMDURATION) {
			simOver = false;
			Customer c;
			c.custID = IDCounter;
			IDCounter++;
			c.arrivalTime = t;
			c.serviceTime = randomPos(SERVICERATE);
			if (strat == oneQ){
				arrivalQueue.enqueue(c);
			} else {
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
		}
		for (int i = 0; i < NUM_STATIONS; i++){
			longDuration waitTime = t;
			if (stations[i].service()){
				numServed++;
				if (strat == oneQ && arrivalQueue.getSize() > 0){
					Customer c = arrivalQueue.dequeue();
					waitTime -= c.arrivalTime;
					stations[i].queue.enqueue(c);
					totalWaitTime += waitTime;
					if (waitTime > maxWaitTime){
						maxWaitTime = waitTime;
					}
				} else if (strat != oneQ){
					waitTime -= stations[i].served.arrivalTime;
					stations[i].totalWaitTime += waitTime; //Assumption: "wait time" refers to time in line, and concludes when customer begins checkout	
					if (waitTime > stations[i].maxWaitTime){
						stations[i].maxWaitTime = waitTime;
					}
				}
			} else if(stations[i].time2Serve == 0 && strat == oneQ && arrivalQueue.getSize()){
				Customer c = arrivalQueue.dequeue();
				waitTime -= c.arrivalTime;
				stations[i].served = c;
				stations[i].time2Serve = c.serviceTime - 1;
				totalWaitTime += waitTime;
				if (waitTime > maxWaitTime){
					maxWaitTime = waitTime;
				}
				stations[i].timeBusy++;
			}
			if (t == SIMDURATION && strat != oneQ){
				stations[i].EODlength = stations[i].lineSize();
				stations[i].EODtimeRemaining = stations[i].time2Serve + stations[i].queue.estimateWaitTime();	
			}
		}
		t += 1;
		if (t > SIMDURATION){
			simOver = true;
			for (int i = 0; i < NUM_STATIONS; i++){
				if (stations[i].lineSize() > 0){
					simOver = false;
				}
			}
			if (strat == oneQ){
				if (arrivalQueue.getSize() > 0){
					simOver = false;
				}
			}
		}
	}while (!(simOver));
	std::cout << "\n--Test Parameters--\nStrategy: " << argv[1] << "\nCustomer Arrival Duration (Initial Sim Duration): " << argv[2] << "\nArrival Rate: " << argv[3] << "\nService Rate: " << argv[4];
	std::cout << "\n--RESULTS--\nnumber served: " << numServed << "\nFinal Duration: " << t << std::endl; 
	if (strat == oneQ){
		std::cout << "Average Wait Time: " << totalWaitTime / numServed << "\nMax Wait Time: " << maxWaitTime << std:: endl;
	}
	for (int i = 0; i < NUM_STATIONS; i++) {
		ServiceStation s = stations[i];
		std::cout << "station #" << i << "\n\tbusy ratio: " << (double)s.timeBusy / (double)t << std::endl;
		if (strat != oneQ){
			std::cout << "\tEnd Of Day Line Size: " << s.EODlength << "\n\tEnd Of Day wait time left: " << s.EODtimeRemaining << std::endl;
			std::cout << "\tAverage Wait Time: " << s.totalWaitTime / s.numServed << "\n\tMax Wait Time: " << s.maxWaitTime << std::endl;
		}
	}
}
