#pragma once

class Worker {
private:

	int currentClient; //numer aktualnego klienta	
	void workerLock(); //funkcja okreslajaca zachowanie pracownika (na przerwie czy pracuje)
	void workerWorking(); //funkcja kiedy pojawia sie klient
	
public: 

	void start(); //Rozpoczyna watek pracownika
};
