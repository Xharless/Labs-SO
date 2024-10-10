#include <iostream>
#include <unistd.h> //para el fork
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
    bool juegoTerminado;
};

struct jugador {
    vector <carta> mano;
};

/*
crear_mazo (vector<carta>): Siguiendo las reglas del juego se crean las cartas según color y tipo (numero),
                            además de crear las cartas especiales y las negras, todo esto se agrega en una
                            "lista" mazo que luego será retornada. 

Parámetros:
    Sin parámetros.

Retorno:
    Retorna el mazo creado.
*/
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

/*
es_jugable (bool): Dada la carta de la pila y la carta utilizada por el jugador,
                   si la carta del jugador coincide con el color de la pila/numero o
                   es una carta especial (negro) entonces puede jugarla y retorna true.

Parámetros:
    const carta &cartaPila: carta que está en la pila.
    const carta &cartaJugador: carta utilizada por el jugador.

Retorno:
    Retorna true si la carta empleada por el jugador sigue las reglas del juego, de lo
    contrario false y la carta no se puede jugar.
*/
bool es_jugable(const carta &cartaPila, const carta &cartaJugador){
    return cartaJugador.color == cartaPila.color || cartaJugador.tipo == cartaPila.tipo || cartaJugador.color == "Negro";
}

/*
manejar_carta_negras (void): Permite jugar las cartas negras dependiendo de su tipo, se elige color a cambiar y
                             en caso de que sea +4 hace robar las cartas al siguiente jugador perdiendo su turno,
                             finalmente borra la carta jugada de la mano y se agrega la nueva a la pila de descarte.

Parámetros:
    juego* estadoJuego: contiene mazo jugador, pila de descarte, turno actual.
    int jugadorActual: numero del jugador actual.
    carta jugada: carta especial jugada. 

Retorno:
    Sin retorno.
*/
void manejar_carta_negras(juego* estadoJuego, int jugadorActual, carta jugada){
    string nuevo_color;
    int color_elegido;

    if(jugadorActual == 0){
        cout << "Has jugado una carta "<< jugada.tipo << ". Elige el nuevo color (1 = Amarilla, 2 = Verde, 3 = Rojo, 4 = Azul): ";
        cin >> color_elegido;

        switch(color_elegido){
            case 1: 
                nuevo_color = "Amarillo";
                break;
            case 2:
                nuevo_color = "Verde";
                break;
            case 3: 
                nuevo_color = "Rojo";
                break;
            case 4: 
                nuevo_color = "Azul";
                break;
            default:
                cout << "Opcion invalida, se eligira el color amarillo por defecto" << endl;
                nuevo_color = "Amarillo";
        }
    } else {
        //bot "elige" color
        int color_aleatorio = rand() % 4 + 1;
        switch(color_aleatorio) {
            case 1:
                nuevo_color = "Amarillo";
                break;
            case 2:
                nuevo_color = "Verde";
                break;
            case 3:
                nuevo_color = "Rojo";
                break;
            case 4:
                nuevo_color = "Azul";
                break;
        }
        cout << "Bot " << jugadorActual << " ha cambiado el color a " << nuevo_color << endl;
    }
    
    if(jugada.tipo == "+4"){
        int siguienteJugador = (jugadorActual + 1)%4;
        cout << "El jugador " << siguienteJugador << " debe robar 4 cartas" << endl;
    
        for(int i = 0; i<4; i++){
            if(!estadoJuego->mazo.empty()){
                estadoJuego->manos[siguienteJugador].push_back(estadoJuego->mazo.back());
                estadoJuego->mazo.pop_back();
            } else {
                cout << "El mazo está vacío" << endl;
                break;
            }
        }

        cout << "El jugador " << siguienteJugador << " pierde el turno." << endl;
        estadoJuego->turnoActual = (estadoJuego->turnoActual + 2)%4;

    } else {
        //si es tipo comodin sigue el siguiente jugador
        estadoJuego->turnoActual = (estadoJuego->turnoActual + 1)%4;
    }
   
    //da el color que se selecciono a la pila de descarte
    jugada.color = nuevo_color;
    estadoJuego->pilaDescarte.push_back(jugada);
}

/*
jugar_turno_persona (void): Muestra cartas actuales del jugador y le permite jugar en su turno, si la
                            carta no corresponde a las reglas deberá jugar de nuevo, o robar una carta.

Parámetros:
    juego* estadoJuego: contiene mazo jugador, pila de descarte, turno actual.

Retorno:
    Sin retorno.
*/
void jugar_turno_persona(juego* estadoJuego){
    carta cartaPila = estadoJuego->pilaDescarte.back();
    bool hay_jugable = false;

    cout << "Carta actual de la pila de descarte: " << cartaPila.color << " " << cartaPila.tipo << endl;
    
    cout << "Tus cartas son: \n";
    for(size_t i = 0; i<estadoJuego->manos[0].size(); i++){
        //muestra carta en posesión
        cout << i+1 << ": " << estadoJuego->manos[0][i].color << " " << estadoJuego->manos[0][i].tipo << endl;
    }

    //ver si se puede jugar
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

                if(nuevaCarta.color == "Negro"){
                    manejar_carta_negras(estadoJuego,0,nuevaCarta);
                    estadoJuego->manos[0].pop_back();
                } else{
                    estadoJuego->pilaDescarte.push_back(nuevaCarta);
                    estadoJuego->manos[0].pop_back();
                    estadoJuego->turnoActual = 1; //siguiente turno
                }

            } else {
                cout << "La carta que has robado no es jugable, pasas el turno\n";
                cout << " " << endl;
                estadoJuego->turnoActual = 1;
            }

        } else {
            cout << "El mazo esta vacio. No puedes robar mas cartas\n";
            cout << " " << endl;
            estadoJuego->turnoActual = 1;
        }

    } else {
        int eleccion;
        
        cout << "Elige una carta para jugar (1-" << estadoJuego->manos[0].size() << "): ";
        cin>>eleccion;

        if(eleccion>0 && static_cast<vector<carta>::size_type>(eleccion) <= estadoJuego->manos[0].size()){
            carta seleccionada = estadoJuego->manos[0][eleccion -1];
            if(es_jugable(cartaPila, seleccionada)){
                cout << "Has jugado: " << seleccionada.color << " " << seleccionada.tipo << endl;
                cout << " " << endl;  

                if(seleccionada.color == "Negro"){
                    manejar_carta_negras(estadoJuego,0,seleccionada);
                    estadoJuego->manos[0].erase(estadoJuego->manos[0].begin() + (eleccion - 1));
                } else {
                    estadoJuego->pilaDescarte.push_back(seleccionada); //carta jugada a la pila de descarte
                    estadoJuego->manos[0].erase(estadoJuego->manos[0].begin() + (eleccion - 1)); //se borra la carta del mazo del jugador
                    estadoJuego->turnoActual = 1; //siguiente turno  
                }

            } else {
                cout << "No puedes jugar esa carta, elige una valida\n";
                jugar_turno_persona(estadoJuego);
                return;
            }

        } else {
            cout << "Eleccion invalida, intente otra vez\n";
            jugar_turno_persona(estadoJuego);
            return;
        }
    }
    
    //jugador gana si su mano esta vacia
    if(estadoJuego->manos[0].empty()){
        cout << "Has jugado todas las cartas y ganaste la partida !!" << endl;
        estadoJuego->juegoTerminado = true;
    } 
}

/*
jugar_turno_bot (void): Permite jugar turno del bot, determina si es necesario robar una carta o no y
                        juega la primera carta jugable que tenga.

Parámetros:
    juego* estadoJuego: contiene mazo jugador, pila de descarte, turno actual.
    int num_bot: numero del bot que está jugando.

Retorno:
    Sin retorno.
*/
void jugar_turno_bot(juego* estadoJuego, int num_bot){
    carta cartaPila = estadoJuego->pilaDescarte.back();
    carta jugada;
    bool hay_jugable = false;
    int posicion = 0;

    cout << "Carta actual de la pila de descarte: " << cartaPila.color << " " << cartaPila.tipo << endl;

    //ver si bot puede jugar carta
    for(const auto &cartaJugador : estadoJuego->manos[num_bot]){
        if(es_jugable(cartaPila, cartaJugador)){
            hay_jugable = true;
            jugada = cartaJugador; //obtiene la carta que se va a jugar
            break;
        }
        posicion++;
    }

    if(!hay_jugable){
        cout << "Bot " << num_bot << " no tiene carta jugable" << endl;
        if(!estadoJuego->mazo.empty()){
            carta nuevaCarta = estadoJuego->mazo.back();
            estadoJuego->mazo.pop_back();
            estadoJuego->manos[num_bot].push_back(nuevaCarta);
            cout << "Bot " << num_bot << " ha robado una carta" << endl;

            //ver si la robada es jugable
            if(es_jugable(cartaPila, nuevaCarta)){
                cout << "Bot " << num_bot << " ha robado una carta jugable, se jugara automaticamente" << endl;
                cout << "Jugó " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;
                cout << " " << endl;

                if(nuevaCarta.color == "Negro"){
                    manejar_carta_negras(estadoJuego,num_bot,jugada);
                    estadoJuego->manos[num_bot].pop_back();
                } else{
                    estadoJuego->pilaDescarte.push_back(nuevaCarta);
                    estadoJuego->manos[num_bot].pop_back();
                    estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4; 
                }
                
            } else {
                cout << "Bot " << num_bot << " ha robado una carta no jugable, pasa de turno" << endl;
                cout << " " << endl;
                estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4; 
            }

        } else {
            cout << "El mazo esta vacio. Bot " << num_bot << " no puede robar mas cartas" << endl;
            cout << " " << endl;
            estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4; 
        }

    } else {
        cout << "Bot " << num_bot << " jugó " << jugada.color << " " << jugada.tipo << endl;

        if(jugada.color == "Negro"){
            manejar_carta_negras(estadoJuego,num_bot,jugada);
            estadoJuego->manos[num_bot].erase(estadoJuego->manos[num_bot].begin() + posicion);
        } else {
            estadoJuego->pilaDescarte.push_back(jugada); //carta jugada a la pila de descarte
            estadoJuego->manos[num_bot].erase(estadoJuego->manos[num_bot].begin() + posicion); //se borra la carta del mazo del bot 
            estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4; 
        }
    }
    
    //bot gana la partida al tener su mano vacia
    if(estadoJuego->manos[num_bot].empty()){
        cout << "Bot " << num_bot << " jugó todas las cartas y gano la partida" << endl;
        estadoJuego->juegoTerminado = true;
    }
}

/*
revolver (void): Dado un mazo, cambia la posición de sus cartas.

Parámetros:
    vector<carta>& mazo: lista de cartas.

Retorno:
    Sin retorno.
*/
void revolver(vector<carta>& mazo){
    srand(time(0));
    for(size_t i = 0; i<mazo.size(); i++){
        size_t j = rand() % mazo.size();
        swap(mazo[i], mazo[j]);    
    }
}


int main(){
    int shmID = shmget(IPC_PRIVATE, sizeof(juego), IPC_CREAT | 0666); //para crear id de memoria compartida
    if(shmID == -1){
        cerr << "Error al crear la memoria compartida";
        exit(1);
    }
    
    //leer y escribir en bloque de memoria, por lo que se adjunta el bloque de memoria del proceso
    juego *estadoJuego = (juego *)shmat(shmID, NULL, 0);
    if (estadoJuego == (void *)-1){
        cerr << "Error al adjuntar la caga de memoria compartida";
        exit(1);
    }

    //iniciar el mazo
    estadoJuego->mazo = crear_mazo();
    revolver(estadoJuego->mazo);
    estadoJuego->manos.resize(4); //para los 4 jugadores
    estadoJuego->turnoActual = 0;
    estadoJuego->juegoTerminado = false;

    //repartir cartas que son 7 para cada jugador
    for(int i = 0; i<4; ++i){
        for(int j = 0; j<7; ++j){
            estadoJuego->manos[i].push_back(estadoJuego->mazo.back());
            estadoJuego->mazo.pop_back();
        }
    }

    //agregamos la primera carta a jugar a la pila de descarte
    estadoJuego->pilaDescarte.push_back(estadoJuego->mazo.back());
    estadoJuego->mazo.pop_back();

    //crear a los jugadores usando fork
    for(int i = 1; i<=3; ++i){
        pid_t pid = fork();

        if(pid==0){
            //codigo del proceso hijo
            while (true){
                //verificar que el juego ha terminado
                if(estadoJuego->juegoTerminado){
                    break;
                }
                
                //turno del bot
                if(estadoJuego->turnoActual == i){ 
                    jugar_turno_bot(estadoJuego, i);
                    sleep(1);
                }
            }
            return 0;

        } else if (pid < 0){
            cerr << "Error al crear el proceso";
            exit(1);
        }
    }

    //para el proceso padre
    while (true){
        //verificar que el juego ha terminado
        if(estadoJuego->juegoTerminado){
            break;
        }
        
        //turno del jugador
        if(estadoJuego->turnoActual == 0){
            jugar_turno_persona(estadoJuego);
            sleep(1);
        }
    }

    for(int i = 1; i<=3; ++i){
        wait(NULL);
    }

    //libera la memoria compartida
    shmdt(estadoJuego);
    shmctl(shmID, IPC_RMID, NULL);

    return 0;
}