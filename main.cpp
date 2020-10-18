#include <iostream>
#include <thread>
#include <ncurses.h>
#include <mutex>
#include "client.h"
#include "worker.h"
#include "clientFree.h"

using namespace std;
mutex localMutex;

void initWindow() {

	localMutex.lock();
    move(0,0);
    printw("SYMULATOR MYJNI SAMOCHODOWEJ");
    refresh();
	move(2,0);
	printw("PARKING:");
	refresh();
	move(7,0);
	printw("Info:");
	refresh();
	move(9,0);
	printw("--------------------------------------------");
	refresh();
	move(10,0);
	printw("STANOWISKA BEZOBSLUGOWE");

	for(int i=0; i<4; i++) {

		move(12+i,0);
		clrtoeol();
		printw("Stanowisko %d jest: wolne",i+1);
		refresh();
	}
	move(17,0);
	printw("Info:");
	refresh();
	localMutex.unlock();
}

int main()
{
	initscr();
    cbreak();
    start_color();
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    attron(COLOR_PAIR(1));
	initWindow();

  	int totalPlaces=3;
	int NO_clients=30;
	int freeClients=20;

	//Utworzenie obiektow
	Worker worker;
	Client client;
	FreeClient freeclient;

	//Utworzenie watkow
	thread workerThread(&Worker::start, worker);
	thread clientThread(&Client::generateNewClient, client, totalPlaces, NO_clients);
	thread freeClientThread(&FreeClient::newClient, freeclient);

	workerThread.join();
	clientThread.join();
	freeClientThread.join();

	endwin();
    exit(0);
}