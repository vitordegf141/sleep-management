#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <iostream>
#include <vector>

using namespace std;

#define PORT 4000
#define PORT_CLIENT 4021
#define BUFFER_SIZE 1024
#define MONITORING_PORT 8889

// Struct que armazena os dados de cada participante
struct ParticipantData {
    string Hostname;
    string MAC;
    string ip_address;
    bool is_awaken;
};

// A tabela é representada por uma lista de structs
vector<ParticipantData> ParticipantsTable;

// Variáveis globais
bool isManager = false;
int numParticipants = 0;

pthread_mutex_t participantsMutex;
pthread_rwlock_t participantsRWLock;
pthread_mutex_t tableMutex = PTHREAD_MUTEX_INITIALIZER;


// Executado pelo manager
// Identifica quais computadores passaram a executar o programa, recebendo e respondendo pacotes em broadcast
class discovery_subservice{

    
};

int createSocket(int port, char serverName[]) {
        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        int yes = 1;
        int ret;
        struct hostent *server;
        if (sockfd < 0) {
            std::cerr << "Failed to create socket." << std::endl;
            exit(EXIT_FAILURE);
        }

        ret = setsockopt(sockfd,SOL_SOCKET,SO_BROADCAST,&yes,sizeof(yes));
        if(ret != 0)
        {
            std::cerr << "Failed to configure the socket." << std::endl;
            exit(EXIT_FAILURE);
        }
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        if(serverName == NULL)
            serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        else{
            printf("getting host name %s\n", serverName);
            server = gethostbyname(serverName);
            if(server == NULL)
            {
                printf("failed to find host;\n");
            }
            serverAddr.sin_addr = *((struct in_addr *)server->h_addr);
        }
        bzero(&(serverAddr.sin_zero), 8); 
        if(serverName == NULL)
            if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
                std::cerr << "Failed to bind the socket." << std::endl;
                close(sockfd);
                exit(EXIT_FAILURE);
            }
        printf("Socket id = %d",sockfd);
        std::cout << "\tDiscovery Socket Created\n" << std::endl;
        return sockfd;
    }

void *discoveryThread(void *arg) {
        std::cout << "Starting Discovery\n" << std::endl;
        int sockfd = createSocket(PORT,NULL);
        struct sockaddr_in clientAddr;
        char buffer[BUFFER_SIZE];
        printf("before while\n");
        socklen_t len = sizeof(clientAddr);
        while (1) {
            std::cout << "while Discovery\n" << std::endl;
            printf("in while\n");
            memset(buffer, 0, BUFFER_SIZE);
            printf("after memset\n");
            std::cout << "while Discovery\n" << std::endl;
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&clientAddr, &len);
            printf(" n= %d",n);
            std::cout << "no n \n" << std::endl;
            if (n > 0) {
                std::cout << "received packaged" << std::endl;
                buffer[n] = '\0';
                printf("message = %s",buffer);
                std::cout << "\nmessage^\n" << std::endl;
                // Extract the hostname from the buffer
                char* hostname = strtok(buffer, ",");
                if (hostname != NULL) {
                    // Extract the status from the buffer
                    printf("hostname received = %s\n",hostname);
                    char* status = strtok(NULL, ",");
                    if (status != NULL) {
                        int i;
                        for (i = 0; i < ParticipantsTable.size(); i++) {
                            ParticipantData* participant = &ParticipantsTable[i];
                        }
                    }
                }
                
            }
        }
    }

int sendDiscoverypackaged()
{
    unsigned int length;
    
    char MyMac[] = "AA:BB:CC:DD:EE:FF";
    char buffer[BUFFER_SIZE];
    char serv_addr[] = "255.255.255.255"; // MY BROADCAST IP
    int send,receive;
    struct hostent *server;
    int sockfd = createSocket(PORT_CLIENT,serv_addr);
    length = sizeof(struct sockaddr_in);
    struct sockaddr_in serveraddress;
    serveraddress.sin_family = AF_INET;     
	serveraddress.sin_port = htons(PORT);    
    server = gethostbyname(serv_addr);
    if(server == NULL)
    {
        printf("problem finding master server");
    }
	serveraddress.sin_addr = *((struct in_addr *)server->h_addr);
    receive = 0; //Mudar para receber confirmacao
    do{
        printf("sending discovery packaged");
        send = sendto(sockfd, MyMac, strlen(MyMac), 0, (const struct sockaddr *) &serveraddress, sizeof(struct sockaddr_in));
        if (send < 0) 
            printf("\nERROR sendto: %d \n",send);
        else
        {
             //Mudar para receber confirmacao
            //receive = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &serveraddress, &length);
            //printf("received discovery packaged: %s",buffer);
        }
        
    }while(send >=0 && receive >=0);
    
    return 0;
}
// Executado pelo manager
// Monitora o status dos computadores a partir do envio de pacotes
class monitoring_subservice{
public:
    // void* monitoringService(void* arg) {
    //     discovery_subservice discovery;
    //     int sockfd = discovery.createSocket(MONITORING_PORT);
    //     struct sockaddr_in clientAddr;
    //     char buffer[BUFFER_SIZE];

    //     while (1) {
    //         // Send sleep status request to participants
    //         int i;
    //         for (i = 0; i < ParticipantsTable.size(); i++) {
    //             // Skip if participant is the manager
    //             if (ParticipantsTable[i].Hostname == "manager")
    //                 continue;

    //             // Prepare sleep status request packet
    //             char request[BUFFER_SIZE];
    //             snprintf(request, BUFFER_SIZE, "sleep_status_request,%s", ParticipantsTable[i].Hostname);

    //             // Create socket address for participant
    //             struct sockaddr_in participantAddr;
    //             memset(&participantAddr, 0, sizeof(participantAddr));
    //             participantAddr.sin_family = AF_INET;
    //             participantAddr.sin_port = htons(MONITORING_PORT);
    //             // participantAddr.sin_addr.s_addr = inet_addr(ParticipantsTable[i].ip_address);

    //             // Send sleep status request packet to participant
    //             sendto(sockfd, (const char*)request, strlen(request), MSG_CONFIRM, (const struct sockaddr*)&participantAddr, sizeof(participantAddr));
    //         }

    //         sleep(5); // Wait for sleep status responses
    //     }
    // }

};



// Executado pelo manager
// Mantém uma lista dos computadores participantes com o status de cada um
class management_subservice{
public:

    void AddParticipantToTable(string hostname, string mac, string ip, bool is_awaken){
        
        ParticipantData new_participant;
        new_participant.Hostname = hostname;
        new_participant.MAC = mac;
        new_participant.ip_address = ip;
        new_participant.is_awaken = is_awaken;

        try{
            // Adiciona participante no final da lista
            ParticipantsTable.push_back(new_participant);
            std::cout << "Participant " << hostname <<" added to table\n";
        }
        catch(std::exception& e){
            std::cout << "Error adding participant to table: \n" << e.what();
        }
    }
    
    void UpdateParticipantStatus(string hostname, bool is_awaken)
    {
        for (int i; i<ParticipantsTable.size(); i++){
            if (ParticipantsTable[i].Hostname == hostname){
                ParticipantsTable[i].is_awaken = is_awaken;

                break; // Se encontrou, encerra o loop antes
            }
        }
    }

    void RemoveParticipantFromTable(string hostname)
    {
        int indice;
        for (int i; i<ParticipantsTable.size(); i++){
            if (ParticipantsTable[i].Hostname == hostname){
                indice = i;
                
                break; // Se encontrou, encerra o loop antes
            }
        }

        try{
            // Exclui participante pelo seu índice na lista
            ParticipantsTable.erase(ParticipantsTable.begin() + indice);
            std::cout << "Participant " << hostname <<" removed from table";
        }
        catch(std::exception& e){
            std::cout << "Error removing participant from table: " << e.what();
        }
    }
};

// Executado pelo manager e pelo participante
// Exibe informações dos computadores e permite input dos usuários
class interface_subservice{
public:

    // Função para exibir a lista de participantes na tela
    void displayParticipants() {
        pthread_mutex_lock(&tableMutex);
        std::cout << "Participants:" <<  std::endl;
        for (int i=0; i<ParticipantsTable.size(); i++) {
            std::cout << "Hostname: " << ParticipantsTable[i].Hostname << ", IP: " << ParticipantsTable[i].ip_address
                      << ", MAC: " << ParticipantsTable[i].MAC << ", Status: " << ParticipantsTable[i].is_awaken << std::endl;
        }
        pthread_mutex_unlock(&tableMutex);
    }


    void    interfaceThread(void *arg) {
        std::string command;
        std::cout << "Digite o comando: " << std::endl;


        while (1) {
            std::cin >> command;

            if (command == "EXIT") {
                if (isManager) {
                    pthread_rwlock_wrlock(&participantsRWLock);
                    std::cout << "Estação Manager está saindo do serviço" << std::endl;
                    pthread_rwlock_unlock(&participantsRWLock);
                } else {
                    std::cout << "Enviando mensagem de saída..." << std::endl;
                    // Enviar pacote de descoberta especial para indicar saída do serviço
                }

                break;
            } else if (command == "WAKEUP") {
                std::string hostname;
                std::cin >> hostname;
                std::cout << "Enviando comando de WAKEUP para a estação" << hostname << std::endl;
                // Enviar comando de WAKEUP (pacote WoL) para a estação especificada
            } else {
                std::cout << "Comando inválido." << std::endl;
            }
        }
    }
};


int main(int argc, char *argv[]){

    pthread_t discoveryThreadId, interfaceThreadId, monitoringThreadId, managementThreadId;

    if (argc > 1 && strcmp(argv[1], "manager") == 0) {
        isManager = true;
        discovery_subservice discovery;
        std::cout << "Estação iniciada como Manager\n" << std::endl;
        pthread_create(&discoveryThreadId, NULL, discoveryThread, NULL);
        while(1)
        {

        }
    } else {
        std::cout << "Estação iniciada como Participante\n" << std::endl;
            std::string hostname = "MyComputer";
            std::string mac = "AA:BB:CC:DD:EE:FF";
            std::string ip = "192.168.1.100";
            bool is_awaken = true;
            sendDiscoverypackaged();
            // Adicionar o participante na tabela
            management_subservice management;
            management.AddParticipantToTable(hostname, mac, ip, is_awaken);

            // Mostrar participantes
            interface_subservice interface;

            interface.displayParticipants();
    }

    // Adicionar na tabela o novo participante
    //
 
};