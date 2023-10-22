#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <semaphore.h>
#include <algorithm>
#include <random>
#include <map>
#include <iterator>
#include <signal.h>

using namespace std;

mutex technicien;       // Le technicien représente le cuisinier dans le problème
mutex machineACafe;     // La machine à café représente le pot dans le problème
sem_t semaphoreMachineACafe;    // Le sémaphore de la machine à caféf permet de savoir quand la machine a plus de café
int nombreCapsuleDeCafe;        // Nombre de capsule actuellement dans la machine
unsigned capaciteCafeDansLaMachine = 3;     // Capacité max de café dans la machine
unsigned nombreEmploye = 6;     // Les employé représentent les sauvages dans le problème

map<thread::id, atomic_uint> mapNombreCafeParEmploye;   // Map permettant de savoir combien de café a bu un employé

// Procédure pour simuler l'appel du technicien
void appelTechnicien() {
    // On bloque le technicien
    technicien.lock();

    cout << "Le technicien remet du café dans la machine" << endl;
    this_thread::sleep_for(1000ms);     // On attend qu'il répare la machine

    // On post le sémaphore pour simuler le fait de rajouter du café dans la machine
    for (unsigned i = 0; i < capaciteCafeDansLaMachine; i += 1) {
        // On poste le sémaphore
        sem_post(&semaphoreMachineACafe);
    }

    cout << "La machine est de nouveau opérationnelle" << endl;

    // On libère le technicien
    technicien.unlock();
}

// Procédure pour simuler le fait que l'employé consomme le café
void boisCafe() {

    cout << "Miam le café est bon" << endl;
    // On wait le sémaphore pour simuler la consommation du café
    sem_wait(&semaphoreMachineACafe);
    // On incrémente l'indice de l'employé qui a pris son café
    mapNombreCafeParEmploye[this_thread::get_id()] += 1;

    // On libère la machine
    machineACafe.unlock();
}

// Procédure pour simuler le fait qu'un employé va prendre un café
void serviceMachine() {
    // On bloque la machine à café
    machineACafe.lock();
    cout << "L\'employe n°" << this_thread::get_id() << " va prendre un café" << endl;
    // On récupère la valeur dans le sémaphore
    sem_getvalue(&semaphoreMachineACafe, &nombreCapsuleDeCafe);
    cout << nombreCapsuleDeCafe << " capsule(s) restante(s)" << endl;
    // S'il n'y a plus de capsule à café dans la machine
    if (nombreCapsuleDeCafe == 0) {
        // On appelle le technicien
        appelTechnicien();
    }
    // L'employé boit le café
    boisCafe();
    cout << "La machine est libre" << endl;

}

int main() {
    // On initialise le sémaphore
    sem_init(&semaphoreMachineACafe, 1, capaciteCafeDansLaMachine);

    // On initialise le vecteur de threads
    vector<thread> employes;
    // On initialise les threads en remplissant le vecteur
    for (unsigned i = 0; i < nombreEmploye; i += 1) {
        employes.emplace_back(serviceMachine);

        // On met l'employé dans la map et on initialise son nombre de café à 0
        mapNombreCafeParEmploye[employes[i].get_id()] = 0;
    }


    auto aleatoire = default_random_engine{};

    /* La partie avec le gestionnaire du signal est inspiré du gestionnaire du groupe sur Cigarette Smoker Problem (Dugourd - Cazals - Leiner - Gonzales) */
    // Si on arrête le programme
    signal(SIGINT, [](int) {
        unsigned nombre = 1;
        // On affiche le nombre de café par employé
        for (auto const &pair : mapNombreCafeParEmploye) {
            cout << "L\'employe n°" << nombre << " a mangé " << pair.second << " fois." << endl;
            nombre += 1;
        }
        exit(0);
    });

    // Le programme tourne en boucle, il faut donc l'interrompre avec CTRL+C
    while (true) {
        cout << "=== NOUVEAU ===" << endl;
        // On mélange le vecteur avec les employés
        shuffle(employes.begin(), employes.end(), aleatoire);
        for (auto & employe : employes) {
            // On lance les threads
            if (employe.joinable()) {
                employe.join();
                employe = thread(serviceMachine);
            }
        }
    }

    return 0;
}
