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
    int direccionTurno; // 1 para adelante, -1 para atras
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
        cout << "Has jugado una carta "<< jugada.tipo << ". Elige el nuevo color (1 = Amarillo, 2 = Verde, 3 = Rojo, 4 = Azul): ";
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
manejar_carta_especial (void): Permite jugar las cartas especiales (salta, +2 y cambio sentido),
                               dependiendo del tipo de carta especial se actualizará el estadoJuego.
                               Salta cambia el turnoActual de forma que salta al siguiente, +2 saca dos
                               cartas del mazo y se las entrega al jugador correspondiente, pasando de turno,
                               cambio sentido voltea los turnos.

Parámetros:
    juego* estadoJuego: contiene mazo jugador, pila de descarte, turno actual.
    int jugadorActual: numero del jugador actual.
    carta cartaEspecial: carta especial jugada. 

Retorno:
    Sin retorno.
*/
void manejar_carta_especial(juego* estadoJuego, carta cartaEspecial, int jugadorActual) {
    int siguienteJugador = (estadoJuego->turnoActual + 1) % 4;

    if (cartaEspecial.tipo == "salta") {
        cout << "El jugador " << siguienteJugador << " ha sido saltado." << endl;
        estadoJuego->turnoActual = (estadoJuego->turnoActual + 2) % 4;  //salta el siguiente jugador

    } else if (cartaEspecial.tipo == "+2") {
        cout << "El jugador " << siguienteJugador << " debe robar 2 cartas." << endl;
        
        //agregar dos cartas del mazo a la mano del jugador correspondiente
        for (int i = 0; i < 2; i++) {
            if (!estadoJuego->mazo.empty()) {
                estadoJuego->manos[siguienteJugador].push_back(estadoJuego->mazo.back());
                estadoJuego->mazo.pop_back();
            } else {
                cout << "El mazo está vacío." << endl;
                break;
            }
        }
        estadoJuego->turnoActual = siguienteJugador;

    } else if (cartaEspecial.tipo == "cambio_sentido") {
        cout << "La dirección del juego ha cambiado." << endl;
        estadoJuego->direccionTurno *= -1;  //cambia la dirección del juego
    }
}

/*
jugar_turno_persona (void): Muestra cartas actuales del jugador y le permite jugar en su turno, si la
                            carta no corresponde a las reglas deberá jugar de nuevo, o robar una carta.

Parámetros:
    juego* estadoJuego: contiene mazo jugador, pila de descarte, turno actual.

Retorno:
    Sin retorno.
*/
void jugar_turno_persona(juego* estadoJuego) {
    carta cartaPila = estadoJuego->pilaDescarte.back();
    bool hay_jugable = false;

    cout << "Carta actual de la pila de descarte: " << cartaPila.color << " " << cartaPila.tipo << endl;

    cout << "Tus cartas son:\n";
    for (size_t i = 0; i < estadoJuego->manos[0].size(); i++) {
        cout << i + 1 << ": " << estadoJuego->manos[0][i].color << " " << estadoJuego->manos[0][i].tipo << endl;
    }

    //ver si tiene carta jugable
    for (const auto& cartaJugador : estadoJuego->manos[0]) {
        if (es_jugable(cartaPila, cartaJugador)) {
            hay_jugable = true;
            break;
        }
    }

    //no hay jugable
    if (!hay_jugable) {
        cout << "No tienes carta jugable, roba una carta\n";

        if (!estadoJuego->mazo.empty()) {
            carta nuevaCarta = estadoJuego->mazo.back();
            estadoJuego->mazo.pop_back();
            estadoJuego->manos[0].push_back(nuevaCarta);
            cout << "Has robado: " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;

            //ver si la carta robada es jugable
            if (es_jugable(cartaPila, nuevaCarta)) {
                cout << "Has robado una carta jugable, se jugará automáticamente\n";

                if (nuevaCarta.color == "Negro") {
                    manejar_carta_negras(estadoJuego, 0, nuevaCarta); //juega especial negra

                } else if (nuevaCarta.tipo == "salta" || nuevaCarta.tipo == "+2" || nuevaCarta.tipo == "cambio_sentido"){
                    manejar_carta_especial(estadoJuego, nuevaCarta, 0); //juega especial

                } else {
                    estadoJuego->pilaDescarte.push_back(nuevaCarta);
                    estadoJuego->turnoActual = 1;
                }
                estadoJuego->manos[0].pop_back();  //elimina la carta robada

            } else {
                cout << "La carta que has robado no es jugable, pasas el turno\n";
                estadoJuego->turnoActual = 1;
            }

        } else {
            cout << "El mazo está vacío. No puedes robar más cartas\n";
            estadoJuego->turnoActual = 1;
        }

    //si hay jugable
    } else {
        int eleccion;
        cout << "Elige una carta para jugar (1-" << estadoJuego->manos[0].size() << "): ";
        cin >> eleccion;

        //comprueba que la eleccion esté dentro del largo de la mano
        if (eleccion > 0 && static_cast<vector<carta>::size_type>(eleccion) <= estadoJuego->manos[0].size()) {
            carta seleccionada = estadoJuego->manos[0][eleccion - 1];

            if (es_jugable(cartaPila, seleccionada)) {
                cout << "Has jugado: " << seleccionada.color << " " << seleccionada.tipo << endl;

                if (seleccionada.color == "Negro") {
                    manejar_carta_negras(estadoJuego, 0, seleccionada);

                } else if (seleccionada.tipo == "salta" || seleccionada.tipo == "+2" || seleccionada.tipo == "cambio_sentido") {
                    manejar_carta_especial(estadoJuego, seleccionada, 0);

                } else {
                    estadoJuego->pilaDescarte.push_back(seleccionada); //añade a la pila de descarte
                    estadoJuego->turnoActual = 1;
                }

                estadoJuego->manos[0].erase(estadoJuego->manos[0].begin() + (eleccion - 1)); //elimina carta de la mano

            } else {
                cout << "No puedes jugar esa carta, elige una válida\n";
                jugar_turno_persona(estadoJuego);
                return;
            }

        } else {
            cout << "Elección inválida, intenta otra vez\n";
            jugar_turno_persona(estadoJuego);
            return;
        }
    }

    //verifica si el jugador ha ganado
    if (estadoJuego->manos[0].empty()) {
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
void jugar_turno_bot(juego* estadoJuego) {
    int num_bot = estadoJuego->turnoActual;
    carta cartaPila = estadoJuego->pilaDescarte.back();
    carta jugada;
    bool hay_jugable = false;
    int posicion = 0;

    cout << "Carta actual de la pila de descarte: " << cartaPila.color << " " << cartaPila.tipo << endl;

    //ver si el bot puede jugar una carta
    for (const auto &cartaJugador : estadoJuego->manos[num_bot]) {
        if (es_jugable(cartaPila, cartaJugador)) {
            hay_jugable = true;
            jugada = cartaJugador; //guardamos la carta jugable
            break;
        }
        posicion++;
    }

    //no hay jugable
    if (!hay_jugable) {
        cout << "Bot " << num_bot << " no tiene carta jugable" << endl;
        if (!estadoJuego->mazo.empty()) {
            carta nuevaCarta = estadoJuego->mazo.back();
            estadoJuego->mazo.pop_back();
            estadoJuego->manos[num_bot].push_back(nuevaCarta);
            cout << "Bot " << num_bot << " ha robado una carta" << endl;

            //ver si la carta robada es jugable
            if (es_jugable(cartaPila, nuevaCarta)) {
                cout << "Bot " << num_bot << " ha robado una carta jugable, se jugará automáticamente\n";
                cout << "Bot jugó " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;

                if (nuevaCarta.color == "Negro") {
                    manejar_carta_negras(estadoJuego, num_bot, nuevaCarta);  //juega la carta negra
                } else if (nuevaCarta.tipo == "salta" || nuevaCarta.tipo == "+2" || nuevaCarta.tipo == "cambio_sentido") {
                    manejar_carta_especial(estadoJuego, nuevaCarta, num_bot);  //juega la carta especial
                } else {
                    estadoJuego->pilaDescarte.push_back(nuevaCarta);  //agregar la carta a la pila de descarte
                    estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4;
                }

                estadoJuego->manos[num_bot].pop_back();  //eliminar la carta de la mano del bot

            } else {
                cout << "Bot " << num_bot << " ha robado una carta no jugable, pasa de turno\n";
                estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4;
            }

        } else {
            cout << "El mazo está vacío. Bot " << num_bot << " no puede robar más cartas\n";
            estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4;
        }

    //si hay jugable
    } else {
        cout << "Bot " << num_bot << " jugó " << jugada.color << " " << jugada.tipo << endl;

        if (jugada.color == "Negro") {
            manejar_carta_negras(estadoJuego, num_bot, jugada);  //juega la carta negra
        } else if (jugada.tipo == "salta" || jugada.tipo == "+2" || jugada.tipo == "cambio_sentido") {
            manejar_carta_especial(estadoJuego, jugada, num_bot);  //juega carta especial
        } else {
            estadoJuego->pilaDescarte.push_back(jugada);  //agregar la carta jugada a la pila de descarte
            estadoJuego->turnoActual = (estadoJuego->turnoActual + 1) % 4;
        }

        estadoJuego->manos[num_bot].erase(estadoJuego->manos[num_bot].begin() + posicion);  //eliminar la carta de la mano del bot
    }

    //verificar si el bot ha ganado
    if (estadoJuego->manos[num_bot].empty()) {
        cout << "Bot " << num_bot << " jugó todas las cartas y ganó la partida!\n";
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
    sem_init(&estadoJuego->semaforo, 1, 1); //inicializamos semaforo

    //repartir cartas que son 7 para cada jugador
    for(int i = 0; i<4; ++i){
        for(int j = 0; j<7; ++j){
            estadoJuego->manos[i].push_back(estadoJuego->mazo.back());
            estadoJuego->mazo.pop_back();
        }
    }

    //agregamos la primera carta a jugar a la pila de descarte que no sea especial
    for(size_t i = 0; i<estadoJuego->mazo.size(); i++){
        if(estadoJuego->mazo[i].color != "Negro"){
            if(estadoJuego->mazo[i].tipo != "salta" && estadoJuego->mazo[i].tipo != "+2" && estadoJuego->mazo[i].tipo != "cambio_sentido"){
                estadoJuego->pilaDescarte.push_back(estadoJuego->mazo[i]);
                estadoJuego->mazo.erase(estadoJuego->mazo.begin() + i);
                break;
            }
        }
    }
    
    //crear a los jugadores usando fork
    for(int i = 0; i<4; ++i){
        pid_t pid = fork();

        if(pid==0){
            //codigo del proceso hijo

            sem_wait(&estadoJuego->semaforo); //bloquea semaforo
            while (true){

                //verificar que el juego ha terminado
                if(estadoJuego->juegoTerminado){
                    sem_post(&estadoJuego->semaforo); //libera semaforo al salir
                    break;
                }

                if(estadoJuego->turnoActual == 0){
                    jugar_turno_persona(estadoJuego);
                } else {
                    jugar_turno_bot(estadoJuego);
                }

                sleep(1);
            }
            sem_post(&estadoJuego->semaforo); //libera semaforo
            return 0;

        } else if (pid < 0){
            cerr << "Error al crear el proceso";
            exit(1);
        }
    }

    for(int i = 0; i<4; ++i){
        wait(NULL);
    }

    //libera la memoria compartida
    sem_destroy(&estadoJuego->semaforo); //borra semaforo
    shmdt(estadoJuego);
    shmctl(shmID, IPC_RMID, NULL);

    return 0;
}