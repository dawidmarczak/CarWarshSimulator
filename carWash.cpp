#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <string>
#include <condition_variable>
#include "worker.h"
#include "client.h"
#include "clientFree.h"
#include <ncurses.h>
 
using namespace std;

mutex positionLock; //miejsce w myjni
mutex mutexPrint;
mutex mutexWorker; //pracownik - pracuje/na przerwie
mutex local;
mutex local2;
mutex placesMaintanceFree[4]; //mutex na miejsce w myjni bezobslugowej
condition_variable cv; //zmienna warunkowa cv
queue < int > waitList; //kolejka na prakingu

int workerOnBreak = 1; //pracownik na przerwie
int waitingQueue = 0; //ilosc klientow w kolejce
int breaker = 0; //gdy koniec obslugiwania klienta


// --- PRACOWNIK ---

//1. Kontrola czy pracownik jest na przerwie czy pracuje 
void Worker::workerLock() {
	
	unique_lock < mutex > lock(mutexWorker); //zajecie mutexu pracownika
	
	while (workerOnBreak && waitingQueue == 0) { //kiedy nie ma klientow

		mutexPrint.lock();
		move(5,0);
		clrtoeol();
		move(5,0);
		printw("Pracownik jest na przerwie...");
		refresh();
		mutexPrint.unlock();
		 //cout << "\nWorker is on break!" << "\n";

		cv.wait(lock); //zawieszenie dzialania watku az do otrzymania sygnalu od innego watku
	}
}

//2. Obsluga klienta
void Worker::workerWorking() {

 	lock_guard < mutex > lock(positionLock); //zajecie mutexu stanowiska w myjni
	 
	waitingQueue = waitingQueue - 1; 
	currentClient = waitList.front(); //pobranie pierwszego elelementu z kolejki
	
	if (!waitList.empty()) {
		waitList.pop(); 
	}

	//symulacja mycia samochodu przez pracownika
	int washingTime = rand() % 5000 + 1000;
	this_thread::sleep_for(chrono::milliseconds(washingTime));

	//po skonczeniu pracy przy jednym kliencie
	workerOnBreak = 1;

} //Tutaj nastepuje zwolnienie blokady mutexu positionLock


//3. Start watku pracownika
void Worker::start() {

	while (true) {
	
		workerLock(); //poczatkowo pracownik jest na przerwie

		workerWorking(); //po przybyciu klienta

		//Wyswieltanie informacji o danym kliencie
		mutexPrint.lock();
		move(4,0);
		clrtoeol();
		move(5,0);
		printw("Pracownik obsluguje klienta nr: %d", currentClient);
		refresh();
		mutexPrint.unlock();

		//Wyswietlenie aktualnej kolejki na parkingu
		queue < int > parkingQueue = waitList;
		
		for (int i = 0;  i < waitList.size(); i++) {

			int first = parkingQueue.front();

			mutexPrint.lock();
			move(3,i+(i*5));
			printw("%d    ",first);
			clrtoeol();
			refresh();
			mutexPrint.unlock();

			parkingQueue.pop();
		}

		//Gdy skonczyli sie klienci
		if (waitList.size() == 0 && breaker == 1) {

		mutexPrint.lock();
		move(3,0);
		clrtoeol();
		printw("Koniec klientow na dzis!");
		refresh();
		move(7,0);
		clrtoeol();
		mutexPrint.unlock();
		}
	}
}

// --- KLIENT MYJNI OBSLUGOWEJ --- 

//4. Tworzenie kolejnych klientow
void Client::generateNewClient(int totalPlaces, int NO_clients) {

	thread clientThreads[NO_clients];
	Client client;

	//Utworzenie nowego watku klienta
	arrivalTime = rand() % 7000 + 1000;
	for (int i = 0; i < NO_clients; i++) {
		this_thread::sleep_for(chrono::milliseconds(arrivalTime)); 
		clientThreads[i] = thread(&Client::waitOrLeave, client, i + 1, totalPlaces);
	}

	//Koncowe zlaczenie watkow
	for (int i = 0; i < NO_clients; i++) {
		clientThreads[i].join();

		if (i == NO_clients-1) {
			breaker = 1;
		}
	}

	
}

//5. Zachowanie klienta gdy nie ma miejsca na parkingu
void Client::leave(int clientID) {

	//lock_guard<mutex> lock(mutexPrint);

	int lastClient = waitList.back();
	
	if (clientID - (lastClient + 1) > 0) {
	
		local.lock();
		move(7,0);
		clrtoeol();
		move(7,0);
		printw("Info: *** Klienci od %d do %d odjechali z powodu braku miejsca ***",lastClient+1, clientID);
		local.unlock();
	}
	else {

		local.lock();
		move(7,0);
		clrtoeol();
		move(7,0);
		printw("Info: *** Klient o nr. %d odjechal z powodu braku miejsca *** ",clientID);
		local.unlock();

	}
}

//6. Dodanie klienta do kolejki, jesli jest miejsce
void Client::waitInQueue(int queueNumber) {

	unique_lock < mutex > lock(mutexWorker);

	waitingQueue = waitingQueue + 1;
	workerOnBreak = 0;

	//Dodanie klienta do listy
	if (waitingQueue > 0) {
		waitList.push(queueNumber);
	}
}


//7. Zachowanie klienta - czy czeka czy odjezdza
void Client::waitOrLeave(int id, int totalPlaces) {

	clientID = id;
	parkingPlaces = totalPlaces;

	if (waitingQueue >= parkingPlaces) {
		leave(clientID);
	} else {
		waitInQueue(clientID);
		cv.notify_all();
	}
}


// --- MYJNIA BEZOBSLUGOWA --- 
 void FreeClient::newClient(){

	thread freeClientThreads[20];
	FreeClient freeClient;

	//Utworzenie nowego watku klienta co 1-7s

	generateTime = rand() % 2000 + 1000;
	for (int i = 0; i < 20; i++) {
		this_thread::sleep_for(chrono::milliseconds(generateTime)); 
		freeClientThreads[i] = thread(&FreeClient::washOrLeave, freeClient, i + 1);
	}

	//Koncowe zlaczenie watkow
	for (int i = 0; i < 20; i++) {
		freeClientThreads[i].join();
	}
}

void FreeClient::washOrLeave(int id) {

	int clientID = id;
	//zmienna globalna mutex - mutex placesMaintanceFree[4]; //mutex na miejsce w myjni bezobslugowej

	if(placesMaintanceFree[0].try_lock()) {
			
			lockPosition(0);
	} 
	else if(placesMaintanceFree[1].try_lock()){

			lockPosition(1);
	}
	else if(placesMaintanceFree[2].try_lock()){

			lockPosition(2);
	}
	else if(placesMaintanceFree[3].try_lock()){

			lockPosition(3);
	}
	else {

			local2.lock();
			move(17,0);
			printw("Info: Klient nr. %d odjechal poniewaz nie bylo wolnego stanowska", clientID);
			refresh();
			local2.unlock();
		}
		
}

void FreeClient::lockPosition(int position) {


	local2.lock();
	move(12+position,0);
	clrtoeol();
	printw("Stanowisko %d jest: zajete",position+1);
	refresh();
	local2.unlock();

	//losowy czas mycia samochodu
	int washingTime = rand() % 10000 + 5000;
  	std::this_thread::sleep_for(std::chrono::milliseconds(washingTime));
			
	//zwolnienie stanowiska
	placesMaintanceFree[position].unlock();

	local2.lock();
	move(12+position,0);
	clrtoeol();
	printw("Stanowisko %d jest: wolne",position+1);
	refresh();
	local2.unlock();	
}