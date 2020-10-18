class FreeClient{

private:
    int id; //ID cliena do myjni bezobslugowej
    int washPlaces; //miejsca
    int generateTime; //co jaki czas bedzie sie pojawial kolejny

    void washOrLeave(int id);
    void lockPosition(int position);
 
public:
    void newClient();
};