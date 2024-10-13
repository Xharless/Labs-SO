#include <iostream>
#include <unistd.h> //para el fork
#include <sys/wait.h> //para el wait()
#include <sys/shm.h> //para la memoria compartida 
#include <semaphore.h> //para el uso de semaforos
#include <algorithm>
#include <string>
#include <ctime>
#include <cstdlib>
using namespace std;

struct carta {
    string color;
    string tipo;
};

struct juego {
    carta mazo[108];
    int mazo_size;
    carta pilaDescarte[108];
    int pila_size;
    carta manos[4][60]; //4 jugadores
    int mano_size[4];
    int turnoActual;
    int direccionTurno; // 1 para adelante, -1 para atras
    bool juegoTerminado;
    sem_t semaforo; //crea semaforo
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
void crear_mazo(juego* estadoJuego) {
    string colores[] = {"Amarillo", "Verde", "Rojo", "Azul"};
    string especiales[] = {"salta", "+2", "cambio_sentido"};
    estadoJuego->mazo_size = 0;  

    // Crear cartas numéricas
    for (const string& color : colores) {
        carta nuevaCarta;
        nuevaCarta.color = color;
        nuevaCarta.tipo = "0";
        estadoJuego->mazo[estadoJuego->mazo_size++] = nuevaCarta;

        for (int i = 1; i <= 9; i++) {
            carta cartaNumerica;
            cartaNumerica.color = color;
            cartaNumerica.tipo = to_string(i);
            estadoJuego->mazo[estadoJuego->mazo_size++] = cartaNumerica;
            estadoJuego->mazo[estadoJuego->mazo_size++] = cartaNumerica;
        
        }
    }

    // Crear cartas especiales por cada color
    for (const string& color : colores) {
        for (const string& especial : especiales) {
            carta cartaEspecial;
            cartaEspecial.color = color;
            cartaEspecial.tipo = especial;
            estadoJuego->mazo[estadoJuego->mazo_size++] = cartaEspecial;
            estadoJuego->mazo[estadoJuego->mazo_size++] = cartaEspecial;
            
        }
    }

    // Crear cartas negras que son el comodín y el +4
    for (int i = 0; i < 4; i++) {
        if (estadoJuego->mazo_size < 108) {
            carta cartaComodin;
            cartaComodin.color = "Negro";
            cartaComodin.tipo = "comodin";
            estadoJuego->mazo[estadoJuego->mazo_size++] = cartaComodin;

            carta cartaPlus;
            cartaPlus.color = "Negro";
            cartaPlus.tipo = "+4";
            estadoJuego->mazo[estadoJuego->mazo_size++] = cartaPlus;
        }
    }
}

//si no salta entonces salta=0, si salta entonces salta=1
void turno_siguiente(juego* estadoJuego, int salta){

    //en caso de que turno actual sea 0 y deba restarse entonces sumar 4
    if(estadoJuego->turnoActual == 0 && estadoJuego->direccionTurno == -1){
        estadoJuego->turnoActual = (estadoJuego->turnoActual + 4 - salta + estadoJuego->direccionTurno) % 4;

    } else if (estadoJuego->turnoActual != 0 && estadoJuego->direccionTurno == -1){
        //se resta el salto si el turno es en reversa
        estadoJuego->turnoActual = (estadoJuego->turnoActual - salta + estadoJuego->direccionTurno) % 4;

    } else{
        //si el turno no es en reversa entonces se suma el salto
        estadoJuego->turnoActual = (estadoJuego->turnoActual + salta + estadoJuego->direccionTurno) % 4;
    }
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
        cout << "Jugador " << jugadorActual + 1 << " ha cambiado el color a " << nuevo_color << endl;
        cout << " " << endl;
    }

    if(jugada.tipo == "+4"){
        int siguienteJugador = (jugadorActual + estadoJuego->direccionTurno + 4)%4;
        cout << "El jugador " << siguienteJugador + 1 << " debe robar 4 cartas" << endl;
        cout << " " << endl;
    
        for(int i = 0; i<4; i++){
            if(estadoJuego->mazo_size > 0){
                estadoJuego->manos[siguienteJugador][estadoJuego->mano_size[siguienteJugador]] = estadoJuego->mazo[estadoJuego->mazo_size - 1];
                estadoJuego->mazo_size--;
                estadoJuego->mano_size[siguienteJugador]++;

                //estadoJuego->mazo.pop_back();
            } else {
                cout << "El mazo está vacío" << endl;
                cout << " " << endl;
                break;
            }
        }

        cout << "El jugador " << siguienteJugador + 1 << " pierde el turno." << endl;
        cout << " " << endl;
        siguienteJugador = (siguienteJugador + estadoJuego->direccionTurno + 4) % 4;
        turno_siguiente(estadoJuego,1); //salta el siguiente jugador

    } else {
        //si es tipo comodin sigue el siguiente jugador
        turno_siguiente(estadoJuego,0);
    }

    //da el color que se selecciono a la pila de descarte
    jugada.color = nuevo_color;
    estadoJuego->pilaDescarte[estadoJuego->pila_size] = jugada;
    estadoJuego->pila_size++;
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
int siguienteJugador = (estadoJuego->turnoActual + estadoJuego->direccionTurno + 4) % 4;

    if (cartaEspecial.tipo == "salta") {
        cout << "El jugador " << siguienteJugador + 1 << " ha sido saltado." << endl;
        cout << " " << endl;
        turno_siguiente(estadoJuego, 1); // Salta el siguiente jugador

    } else if (cartaEspecial.tipo == "+2") {
        cout << "El jugador " << siguienteJugador + 1 << " debe robar 2 cartas." << endl;
        cout << " " << endl;
    
        // Agregar dos cartas del mazo a la mano del jugador correspondiente
        for (int i = 0; i < 2; i++) {
            if (estadoJuego->mazo_size > 0) {

                estadoJuego->manos[siguienteJugador][estadoJuego->mano_size[siguienteJugador]] = estadoJuego->mazo[estadoJuego->mazo_size - 1];
                estadoJuego->mazo_size--;
                estadoJuego->mano_size[siguienteJugador]++;
                //estadoJuego->mazo.pop_back();

            } else {
                cout << "El mazo está vacío." << endl;
                cout << " " << endl;
                break;
            }
        }

        // Cambia el turno al siguiente jugador considerando el sentido
        turno_siguiente(estadoJuego, 1); 

    } else if (cartaEspecial.tipo == "cambio_sentido") {
        cout << "La dirección del juego ha cambiado." << endl;
        cout << " " << endl;

        // Cambia la dirección del juego
        estadoJuego->direccionTurno *= -1;  
    
        // Avanza al siguiente jugador en la nueva dirección
        turno_siguiente(estadoJuego, 0);
    }

    // Añadir la carta a la pila de descarte
    estadoJuego->pilaDescarte[estadoJuego->pila_size] = cartaEspecial;
    estadoJuego->pila_size++;
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
    carta cartaPila = estadoJuego->pilaDescarte[estadoJuego->pila_size - 1];
    bool hay_jugable = false;

    cout << "Carta actual de la pila de descarte: " << cartaPila.color << " " << cartaPila.tipo << endl;

    cout << "Tus cartas son:\n";
    for (int i = 0; i < estadoJuego->mano_size[0]; i++) {
        cout << i + 1 << ": " << estadoJuego->manos[0][i].color << " " << estadoJuego->manos[0][i].tipo << endl;
    }

    //ver si tiene carta jugable
    for (int i = 0; i < estadoJuego->mano_size[0]; i++) {
        if (es_jugable(cartaPila, estadoJuego->manos[0][i])) {
            hay_jugable = true;
            break;
        }
    }

    //no hay jugable
    if (!hay_jugable) {
        cout << "No tienes carta jugable, roba una carta\n";

        if (estadoJuego->mazo_size > 0) {
            carta nuevaCarta = estadoJuego->mazo[estadoJuego->mazo_size - 1];
            estadoJuego->mazo_size--;

            estadoJuego->manos[0][estadoJuego->mano_size[0]] = nuevaCarta;
            estadoJuego->mano_size[0]++;
            cout << "Has robado: " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;

            //ver si la carta robada es jugable
            if (es_jugable(cartaPila, nuevaCarta)) {
                cout << "Has robado una carta jugable, se jugará automáticamente\n";
                cout << endl;

                if (nuevaCarta.color == "Negro") {
                    manejar_carta_negras(estadoJuego, 0, nuevaCarta); //juega especial negra

                } else if (nuevaCarta.tipo == "salta" || nuevaCarta.tipo == "+2" || nuevaCarta.tipo == "cambio_sentido"){
                    manejar_carta_especial(estadoJuego, nuevaCarta, 0); //juega especial

                } else {
                    estadoJuego->pilaDescarte[estadoJuego->pila_size] = nuevaCarta;
                    estadoJuego->pila_size++;
                    turno_siguiente(estadoJuego,0);
                }

                estadoJuego->mano_size[0]--; //elimina la carta robada

            } else {
                cout << "La carta que has robado no es jugable, pasas el turno\n";
                cout << " " << endl;
                turno_siguiente(estadoJuego,0);
            }

        } else {
            cout << "El mazo está vacío. No puedes robar más cartas\n";
            cout << " " << endl;
            turno_siguiente(estadoJuego,0);
        }

    //si hay jugable
    } else {
        int eleccion;
        cout << "Elige una carta para jugar (1-" << estadoJuego->mano_size[0] << "): ";
        cin >> eleccion;

        //comprueba que la eleccion esté dentro del largo de la mano
        if (eleccion > 0 && eleccion <= estadoJuego->mano_size[0]) {
            carta seleccionada = estadoJuego->manos[0][eleccion - 1];

            if (es_jugable(cartaPila, seleccionada)) {
                cout << "Has jugado: " << seleccionada.color << " " << seleccionada.tipo << endl;
                cout << " " << endl;

                if (seleccionada.color == "Negro") {
                    manejar_carta_negras(estadoJuego, 0, seleccionada);

                } else if (seleccionada.tipo == "salta" || seleccionada.tipo == "+2" || seleccionada.tipo == "cambio_sentido") {
                    manejar_carta_especial(estadoJuego, seleccionada, 0);

                } else {
                    estadoJuego->pilaDescarte[estadoJuego->pila_size] = seleccionada; //añade a la pila de descarte
                    estadoJuego->pila_size++;
                    turno_siguiente(estadoJuego,0);
                }

                estadoJuego->manos[0][eleccion - 1] = {"", ""}; //borra la carta de la mano
                estadoJuego->mano_size[0]--;

            } else {
                cout << "No puedes jugar esa carta, elige una válida\n";
                cout << " " << endl;
                jugar_turno_persona(estadoJuego);
                return;
            }

        } else {
            cout << "Elección inválida, intenta otra vez\n";
            cout << " " << endl;
            jugar_turno_persona(estadoJuego);
            return;
        }
    }

    //verifica si el jugador ha ganado
    if (estadoJuego->mano_size[0]==0) {
        cout << "Has jugado todas las cartas y ganaste la partida !!" << endl;

        //mostrar las cartas restantes de los demás jugadores
        for(int i=1; i < 4; i++){
            cout << "Jugador " << i << " quedó con " << estadoJuego->mano_size[i] << " cartas\n";
        }

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
    carta cartaPila = estadoJuego->pilaDescarte[estadoJuego->pila_size - 1];
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
        cout << "Jugador " << num_bot + 1 << " no tiene carta jugable" << endl;

        if (estadoJuego->mazo_size > 0) {
            carta nuevaCarta = estadoJuego->mazo[estadoJuego->mazo_size - 1];
            estadoJuego->mazo_size--;

            estadoJuego->manos[num_bot][estadoJuego->mano_size[num_bot]] = nuevaCarta;
            estadoJuego->mano_size[num_bot]++;
            cout << "JUgador " << num_bot + 1 << " ha robado una carta" << endl;

            //ver si la carta robada es jugable
            if (es_jugable(cartaPila, nuevaCarta)) {
                cout << "Jugador " << num_bot + 1 << " ha robado una carta jugable, se jugará automáticamente\n";
                cout << "Jugador jugó " << nuevaCarta.color << " " << nuevaCarta.tipo << endl;
                cout << " " << endl;

                if (nuevaCarta.color == "Negro") {
                    manejar_carta_negras(estadoJuego, num_bot, nuevaCarta);  //juega la carta negra
                } else if (nuevaCarta.tipo == "salta" || nuevaCarta.tipo == "+2" || nuevaCarta.tipo == "cambio_sentido") {
                    manejar_carta_especial(estadoJuego, nuevaCarta, num_bot);  //juega la carta especial
                } else {
                    estadoJuego->pilaDescarte[estadoJuego->pila_size] = nuevaCarta; //agregar la carta a la pila de descarte
                    estadoJuego->pila_size++; 
                    turno_siguiente(estadoJuego,0);
                }

                estadoJuego->mano_size[num_bot]--; 
                //estadoJuego->manos[num_bot].pop_back();  //eliminar la carta de la mano del bot

            } else {
                cout << "Jugador " << num_bot + 1 << " ha robado una carta no jugable, pasa de turno\n";
                cout << " " << endl;
                turno_siguiente(estadoJuego,0);
            }

        } else {
            cout << "El mazo está vacío. Jugador " << num_bot + 1 << " no puede robar más cartas\n";
            cout << " " << endl;
            turno_siguiente(estadoJuego,0);
        }

    //si hay jugable
    } else {
        cout << "Jugador " << num_bot + 1 << " jugó " << jugada.color << " " << jugada.tipo << endl;
        cout << " " << endl;

        if (jugada.color == "Negro") {
            manejar_carta_negras(estadoJuego, num_bot, jugada);  //juega la carta negra
        } else if (jugada.tipo == "salta" || jugada.tipo == "+2" || jugada.tipo == "cambio_sentido") {
            manejar_carta_especial(estadoJuego, jugada, num_bot);  //juega carta especial
        } else {
            estadoJuego->pilaDescarte[estadoJuego->pila_size] = jugada; //agregar la carta a la pila de descarte
            estadoJuego->pila_size++;
            turno_siguiente(estadoJuego,0);
        }

        estadoJuego->manos[num_bot][posicion - 1] = {"", ""}; //eliminar la carta de la mano del bot
        estadoJuego->mano_size[num_bot]--;
    }

    //verificar si el bot ha ganado
    if (estadoJuego->mano_size[num_bot] == 0) {
        cout << "Jugador " << num_bot + 1 << " jugó todas las cartas y ganó la partida!\n";

        //mostrar las cartas restantes de los demás jugadores
        for(int i=0; i < 4; i++){
            if(num_bot != i){
                cout << "Jugador " << i + 1 << " quedó con " << estadoJuego->mano_size[i] << " cartas\n";
            }
        }

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
void revolver(carta mazo[], int mazo_size){
    srand(time(0));
    random_shuffle(mazo, mazo + mazo_size);
}

void repartir_cartas(juego* estadoJuego){
    //repartir cartas que son 7 para cada jugador
    for(int i = 0; i<4; ++i){
        for(int j = 0; j<7; ++j){
            estadoJuego->manos[i][j] = estadoJuego->mazo[--estadoJuego->mazo_size];
            estadoJuego->mazo_size--;
        }
        estadoJuego->mano_size[i] = 7; //cada uno parte con 7
    }

    //agregamos la primera carta a jugar a la pila de descarte que no sea especial
    for(int i = 0; i<estadoJuego->mazo_size; i++){
        if(estadoJuego->mazo[i].color != "Negro"){
            if(estadoJuego->mazo[i].tipo != "salta" && estadoJuego->mazo[i].tipo != "+2" && estadoJuego->mazo[i].tipo != "cambio_sentido"){
                estadoJuego->pilaDescarte[0] = estadoJuego->mazo[i]; //se agrega a la pila de descarte

                //borramos la carta agregada del mazo
                for(int j = i; j < estadoJuego->mazo_size - 1; j++){
                    estadoJuego->mazo[j] = estadoJuego->mazo[j+1];
                }
                estadoJuego->mazo_size--;

                break;
            }
        }
    }

    estadoJuego->pila_size = 1;
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

    pid_t pid;

    //iniciar el juego 
    crear_mazo(estadoJuego);
    revolver(estadoJuego->mazo, estadoJuego->mazo_size);
    repartir_cartas(estadoJuego);

    estadoJuego->turnoActual = 0;
    estadoJuego->direccionTurno = 1; //turnos hacia adelante (0,1,2,3)
    estadoJuego->juegoTerminado = false;
    sem_init(&estadoJuego->semaforo, 1, 1); //inicializamos semaforo
    
    //crear a los jugadores usando fork
    for(int i = 0; i<4; ++i){
        pid = fork();

        if(pid==0){

            while (!estadoJuego->juegoTerminado){
                sem_wait(&estadoJuego->semaforo); //bloquea semaforo
                
                cout << getpid() << endl;
                if(estadoJuego->turnoActual == 0){
                    jugar_turno_persona(estadoJuego);
                } else {
                    jugar_turno_bot(estadoJuego);
                }

                sleep(1);
                sem_post(&estadoJuego->semaforo); //libera semaforo
            }
            exit(0);


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

