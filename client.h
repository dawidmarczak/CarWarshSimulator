class Client {
private:

	int clientID; //identyfikator kazdego klienta
	int parkingPlaces; //dostepna miejsca na parkingu
	int arrivalTime; //czas co jaki pojawi sie klient

	void waitOrLeave(int id, int totalPlaces); //okresla zachowanie klienta (czeka czy odjezdza)

	void waitInQueue(int queueNumber); //klient czeka w kolejce
	void leave(int totalPlaces); //klient odjezdza

public:

	 void generateNewClient(int totalPlaces, int NO_clients); //stworzenie watku klienta
};