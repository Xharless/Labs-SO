#include <iostream>
#include <unistd.h> //para el fork
#include <sys/wait.h> //para el wait()
#include <sys/shm.h> //para la memoria compartida 
#include <semaphore.h> //para el uso de semaforos
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;

struct carta {
    string color;
    string tipo;
};

struct juego {
    vector<carta> mazo;
    vector<carta> pilaDescarte;
    vector<vector<carta>> manos; //4 jugadores
    int turnoActual;
    bool juegoTerminado;
    sem_t semaforo; //crea semaforo
};

vector <carta> crear_mazo(){
    vector<carta> mazo;
    string colores[] = {"Amarillo","Verde","Rojo","Azul"};
    string especiales[] = {"salta", "+2", "cambio_sentido"};

    for(const string& color: colores){
        mazo.push_back({color, "0"});
        for(int i = 1; i<=9; i++){
            mazo.push_back({color, to_string(i)});
            mazo.push_back({color, to_string(i)});
        }
    }

    for(const string& color:colores){
        for(const string& especial: especiales){
            mazo.push_back({color,especial});
            mazo.push_back({color,especial});
        }
    }

    for(int i = 0; i<4; i++){
        mazo.push_back({"Negro", "comodin"});
        mazo.push_back({"Negro", "+4"});
    }

    return mazo;
}

bool es_jugable(const carta &cartaPila, const carta &cartaJugador){
    return cartaJugador.color == cartaPila.color || cartaJugador.tipo == cartaPila.tipo || cartaJugador.color == "Negro";
}

/* Manejar cartas negras como comodines o +4 */
void manejar_carta_negras(juego* estadoJuego, int jugadorActual, carta jugada){
    string nuevo_color;
    int color_elegido;

    if(jugadorActual == 0){ // jugador humano
        cout << "Has jugado una carta " << jugada.tipo << ". Elige el nuevo color (1 = Amarilla, 2 = Verde, 3 = Rojo, 4 = Azul): ";
        cin >> color_elegido;
        switch(color_elegido){
            case 1: nuevo_color = "Amarillo"; break;
            case 2: nuevo_color = "Verde"; break;
            case 3: nuevo_color = "Rojo"; break;
            case 4: nuevo_color = "Azul"; break;
            default: cout << "Opción inválida, se elige Amarillo por defecto.\n"; nuevo_color = "Amarillo";
        }
    } else { // bot elige color
        int color_aleatorio = rand() % 4 + 1;
        switch(color_aleatorio) {
            case 1: nuevo_color = "Amarillo"; break;
            case 2: nuevo_color = "Verde"; break;
            case 3: nuevo_color = "Rojo"; break;
            case 4: nuevo_color = "Azul"; break;
        }
        cout << "Bot " << jugadorActual << " cambió el color a " << nuevo_color << endl;
    }

    sem_wait(&estadoJuego->semaforo);
    if(jugada.tipo == "+4"){
        int siguienteJugador = (jugadorActual + 1) % 4;
        cout << "El jugador " << siguienteJugador << " debe robar 4 cartas\n";

        for(int i = 0; i < 4; i++){
            if(!estadoJuego->mazo.empty()){
                estadoJuego->manos[siguienteJugador].push_back(estadoJuego->mazo.back());
                estadoJuego->mazo.pop_back();
            } else {
                cout << "El mazo está vacío\n";
                break;
            }
        }

        cout << "El jugador " << siguienteJugador << " pierde el turno.\n";
        estadoJuego->turnoActual = (estadoJuego->turnoActual + 2) % 4;  // El jugador pierde su turno
    } else {
        estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4;  // Comodín normal
    }

    // Cambiar color y poner en la pila de descarte
    jugada.color = nuevo_color;
    estadoJuego->pilaDescarte.push_back(jugada);
    sem_post(&estadoJuego->semaforo);
}

void jugar_turno_persona(juego* estadoJuego){
    sem_wait(&estadoJuego->semaforo);  // Bloqueamos para proteger el estado del juego

    if(estadoJuego->juegoTerminado){
        sem_post(&estadoJuego->semaforo);  // Liberamos el semáforo
        return;  // Si el juego ya terminó, salimos
    }

    carta cartaPila = estadoJuego->pilaDescarte.back();
    bool hay_jugable = false;
    cout << "Carta actual de la pila de descarte: " << cartaPila.color << " " << cartaPila.tipo << endl;
    
    cout << "Tus cartas son: \n";
    for(size_t i = 0; i<estadoJuego->manos[0].size(); i++){
        cout << i+1 << ": " << estadoJuego->manos[0][i].color << " " << estadoJuego->manos[0][i].tipo << endl;
    }

    // Ver si tiene alguna carta jugable
    for(const auto &cartaJugador : estadoJuego->manos[0]){
        if(es_jugable(cartaPila, cartaJugador)){
            hay_jugable = true;
            break;
        }
    }

    if(!hay_jugable){
        cout << "No tienes carta jugable, roba una carta.\n";
        if(!estadoJuego->mazo.empty()){
            carta nuevaCarta = estadoJuego->mazo.back();
            estadoJuego->mazo.pop_back();
            estadoJuego->manos[0].push_back(nuevaCarta);
            cout << "Has robado: " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;

            if(es_jugable(cartaPila, nuevaCarta)){
                cout << "Has robado una carta jugable. Se jugará automáticamente.\n";
                if(nuevaCarta.color == "Negro"){
                    manejar_carta_negras(estadoJuego, 0, nuevaCarta);
                } else {
                    estadoJuego->pilaDescarte.push_back(nuevaCarta);
                }
                estadoJuego->manos[0].pop_back();  // Elimina la carta jugada de la mano
            }
        } else {
            cout << "El mazo está vacío.\n";
        }

    } else {
        int eleccion;
        cout << "Elige una carta para jugar (1-" << estadoJuego->manos[0].size() << "): ";
        cin >> eleccion;

        if(eleccion > 0 && static_cast<size_t>(eleccion) <= estadoJuego->manos[0].size()){
            carta seleccionada = estadoJuego->manos[0][eleccion - 1];
            if(es_jugable(cartaPila, seleccionada)){
                if(seleccionada.color == "Negro"){
                    manejar_carta_negras(estadoJuego, 0, seleccionada);
                } else {
                    estadoJuego->pilaDescarte.push_back(seleccionada);  // Juega la carta
                }
                estadoJuego->manos[0].erase(estadoJuego->manos[0].begin() + (eleccion - 1));  // Quita la carta de la mano
            } else {
                cout << "No puedes jugar esa carta.\n";
            }
        } else {
            cout << "Elección inválida.\n";
        }
    }

    // Ver si el jugador ha ganado
    if(estadoJuego->manos[0].empty()){
        cout << "Has ganado la partida!!\n";
        estadoJuego->juegoTerminado = true;
    }
    sem_post(&estadoJuego->semaforo);  // Liberamos el semáforo
}

/* Funcionamiento similar para el bot */

int main(){
    int shmID = shmget(IPC_PRIVATE, sizeof(juego), IPC_CREAT | 0666);
    if(shmID == -1){
        cerr << "Error al crear la memoria compartida.\n";
        exit(1);
    }
    
    juego *estadoJuego = (juego *)shmat(shmID, NULL, 0);
    if(estadoJuego == (void *)-1){
        cerr << "Error al adjuntar la memoria compartida.\n";
        exit(1);
    }

    estadoJuego->mazo = crear_mazo();
    revolver(estadoJuego->mazo);
    estadoJuego->manos.resize(4);
    estadoJuego->turnoActual = 0;
    estadoJuego->juegoTerminado = false;
    sem_init(&estadoJuego->semaforo, 1, 1);  // Inicializamos el semáforo

    // Repartir cartas
    for(int i = 0; i < 4; ++i){
        for(int j = 0; j < 7; ++j){
            estadoJuego->manos[i].push_back(estadoJuego->mazo.back());
            estadoJuego->mazo.pop_back();
        }
    }

    estadoJuego->pilaDescarte.push_back(estadoJuego->mazo.back());
    estadoJuego->mazo.pop_back();

    for(int i = 0; i < 4; ++i){
        pid_t pid = fork();
        if(pid == 0){
            while (true){
                sem_wait(&estadoJuego->semaforo);
                if(estadoJuego->juegoTerminado){
                    sem_post(&estadoJuego->semaforo);
                    break;
                }

                if(estadoJuego->turnoActual == i){
                    if(i == 0){
                        jugar_turno_persona(estadoJuego);
                    } else {
                        // Implementa jugar_turno_bot similar
                    }
                }
                sem_post(&estadoJuego->semaforo);
                sleep(1);
            }
            return 0;
        }
    }

    for(int i = 0; i < 4; ++i){
        wait(NULL);
    }

    sem_destroy(&estadoJuego->semaforo);
    shmdt(estadoJuego);
    shmctl(shmID, IPC_RMID, NULL);

    return 0;
}
