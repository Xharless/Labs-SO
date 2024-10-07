#include <iostream>
#include <unistd.h> // para el fork y weas
#include <sys/wait.h> //para el wait()
#include <sys/shm.h> //para la memoria compartida 
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
};

struct jugador {
    vector <carta> mano;
};

vector <carta> crear_mazo(){
    vector<carta> mazo;
    string colores[] = {"Amarillo","Verde","Rojo","Azul"};
    string especiales[] = {"salta", "+2", "cambio_sentido"};

    //crear cartas numericas
    for(const string& color: colores){
        mazo.push_back({color, "0"});
        for(int i = 1; i<=9; i++){
            mazo.push_back({color, to_string(i)});
            mazo.push_back({color, to_string(i)});
        }
    }

    //crear cartas especiales por cada color
    for(const string& color:colores){
        for(const string& especial: especiales){
            mazo.push_back({color,especial});
            mazo.push_back({color,especial});
        }
    }

    //crear cartas negras que son el comodin y el +4
    for(int i = 0; i<4; i++){
        mazo.push_back({"Negro", "comodin"});
        mazo.push_back({"Negro", "+4"});
    }

    return mazo;
}


bool es_jugable(const carta &cartaPila, const carta &cartaJugador){
    return cartaJugador.color == cartaPila.color || cartaJugador.tipo == cartaPila.tipo || cartaJugador.color == "Negro";
}

void jugar_turno_persona(juego* estadoJuego){
    cout << "Tus cartas son: \n";
    for(size_t i = 0; i<estadoJuego->manos[0].size(); i++){
        cout << i+1 << ": " << estadoJuego->manos[0][i].color << " " << estadoJuego->manos[0][i].tipo << endl;
    }
    
    cout << "Carta actual de la pila de descarte: " << estadoJuego->pilaDescarte.back().color << " " << estadoJuego->pilaDescarte.back().tipo << endl;

    // ver si se puede jugar 
    carta cartaPila = estadoJuego->pilaDescarte.back();
    bool hay_jugable = false;

    for(const auto &cartaJugador : estadoJuego->manos[0]){
        if(es_jugable(cartaPila, cartaJugador)){
            hay_jugable = true;
            break;
        }
    }

    if(!hay_jugable){
        cout << "No tienes carta jugable, roba una carta\n";
        if(!estadoJuego->mazo.empty()){
            carta nuevaCarta = estadoJuego->mazo.back();
            estadoJuego->mazo.pop_back();
            estadoJuego->manos[0].push_back(nuevaCarta);
            cout << "Has robado: " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;

            //ver si la robada es jugable
            if(es_jugable(cartaPila, nuevaCarta)){
                cout << "Has robado una carta jugable, se jugara automaticamente\n";
                estadoJuego->pilaDescarte.push_back(nuevaCarta);
                estadoJuego->manos[0].pop_back();
            } else {
                cout << "La carta que has robado no es jugable, pasas el turno\n";
            }
        } else {
            cout << "El mazo esta vacio. No puedes robar mas cartas\n";
        }
    } else {
        int eleccion;
        cout << "Elige una carta para jugar (1-" << estadoJuego->manos[0].size() << "): ";
        cin>>eleccion;
        if(eleccion>0 && eleccion <= estadoJuego->manos[0].size()){
            carta seleccionada = estadoJuego->manos[0][eleccion -1];
            if(es_jugable(cartaPila, seleccionada)){
                estadoJuego->pilaDescarte.push_back(seleccionada);
                estadoJuego->manos[0].erase(estadoJuego->manos[0].begin() + (eleccion - 1));
                cout << "Has jugado: " << seleccionada.color << " " << seleccionada.tipo << endl;
            } else {
                cout << "No peudes jugar esa carta, elige una valida\n";
                jugar_turno_persona(estadoJuego);
                return;
            }
        } else {
            cout << "Eleccion invalida, intente otra vez\n";
            jugar_turno_persona(estadoJuego);
            return;
        }
    }

    estadoJuego->turnoActual = 1;
}

void revolver(vector<carta>& mazo){
    srand(time(0));
    for(size_t i = 0; i<mazo.size(); i++){
        size_t j = rand() % mazo.size();
        swap(mazo[i], mazo[j]);    
    }
}



int main(){
    int shmID = shmget(IPC_PRIVATE, sizeof(juego), IPC_CREAT | 0666); //para crear la wea del id de memoria compartida
    if(shmID == -1){
        cerr << "Error añ crear la memoria compartida";
        exit(1);
    }
    
    // leer y escribir en bloque de memoria, por lo que se adjunta el bloque de memoria del proceso
    juego *estadoJuego = (juego *)shmat(shmID, NULL, 0);
    if (estadoJuego == (void *)-1){
        cerr << "Error al adjuntar la caga de memoria compartida"
        exit(1);
    }

    // iniciar el mazo
    estadoJuego->mazo = crear_mazo();
    revolver(estadoJuego->mazo);
    estadoJuego->manos.resize(4); // para los 4 jugadores
    estadoJuego->turnoActual = 0;

    //repartir cartas  que son 7 para cada jugador
    for(int i = 0; i<4; ++i){
        for(int j = 0; j<7; ++j){
            estadoJuego->manos[i].push_back(estadoJuego->mazo.back());
            estadoJuego->mazo.pop_back();
        }
    }


    estadoJuego->pilaDescarte.push_back(estadoJuego->mazo.back());
    estadoJuego->mazo.pop_back();

    //crear a los jugadores usando fork
    for(int i = 1; i<=3; ++i){
        pid_t pid = fork();
        if(pid==0){
            //codigo del proceso hijo
            while (true){
                if(estadoJuego->turnoActual == i){ 
                    // turno del bot, revisar esta wea 
                    if(!estadoJuego->manos[i].empty()){
                        carta jugada = estadoJuego->manos[i].back();
                        estadoJuego->manos[i].pop_back();
                        estadoJuego->pilaDescarte.push_back(jugada);
                        cout << "Bot " << i << " jugo " << jugada.color << " " << jugada.tipo << endl;
                    }
                    estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4;
                    sleep(1);
                }
            }
            return 0;
        } else if (pid < 0){
            cerr << "Error al crear el proceso";
            exit(1);
        }        
    }
    // para el proceso padre
    while (true){
        if(estadoJuego->turnoActual == 0){
            jugar_turno_persona(estadoJuego);
            sleep(1);
        }
    }

    for(int i = 1; i<=3; ++i){
        wait(NULL);
    }
    shmdt(estadoJuego);
    shmctl(shmID, IPC_RMID, NULL);

    /* muestra el mazo revuelto
    
    vector<carta> mazo = crear_mazo();
    revolver(mazo);

    for(const auto& c:mazo){
        cout << c.color << " " << c.tipo << endl;
    }
    */

    return 0;
}