#include <stdio.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

using namespace std;

//ruta de la carpeta con los archivos a mover
const string path = "./archivos_deportes/";

//definicion de funciones
void lista_archivos();
void leer_archivo(const string&);
void mover_archivo(const string&, const string&, const string&, const string&);
void crear_directorio_recursivo(const string&);


int main(){
    lista_archivos();
    return 0;
}

/*
lista_archivos (void): Se abre el directorio (usando path) en el que están los archivos,
                       para posteriormente leerlos llamando a la función leer_archvivo.
                       En caso de que no se abra imprime un mensaje por consola.

                       string path: la ruta pero en string.
                       DIR *direccion: la ruta pero en tipo DIR.
Parámetros:
    Sin parámetros.

Retorno:
    Sin retorno.
*/
void lista_archivos(){
    string nombre_archivo;
    DIR *direccion;

    //verifica que puede abrir el directorio y lee cada archivo dentro de este
    if((direccion = opendir(path.c_str())) != nullptr){ 
        while(dirent *entry = readdir(direccion)){
            nombre_archivo = entry->d_name;

            //excluye la carpeta actual (.) y la anterior (..) 
            if(nombre_archivo != "." && nombre_archivo != ".."){  
                leer_archivo(nombre_archivo);
            }
        }

        closedir(direccion);

    } else {
        cout << "NO SE PUEDE ABRIR EL DIRECTORIO" << endl; 
    }
}


/*
leer_archivo (void): Se abre el archivo de texto y se obtienen sus tres primeras lineas.
                     Luego llama a la función mover_archivo.
                     En caso de que no pueda abrirlo imprime un mensaje por consola.

Parámetros:
    nombre_archivo (string): nombre del archivo a leer.

Retorno:
    Sin retorno.
*/
void leer_archivo(const string& nombre_archivo){
    string direccion_archivo = path + nombre_archivo; //a la ruta le agregamos el nombre del archivo
    ifstream file(direccion_archivo);

    //leemos el archivo
    if (file.is_open() ){
        string deporte, categoria, medalla;

        //obtenemos las 3 lineas del archivo para el nombre de las carpetas
        getline(file, deporte);
        getline(file, categoria);
        getline(file, medalla);

        mover_archivo(deporte, categoria, nombre_archivo, medalla);

        file.close();

    } else {
        cout << "NO SE PUDO ABRIR EL ARCHIVO: " << direccion_archivo << endl;
    }
}


/*
mover_archivo (void): Se llama a la función crear_directorio_recursivo para crear el directorio
                      base al cual se crearán las carpetas y moveran los archivos, luego comprueba
                      si obtuvo medalla para crear o mover el archivo según corresponda. En caso de
                      que no pueda mover el archivo imprimirá un mensaje por consola.

Parámetros:
    deporte (string): deporte realizado por el participante.
    categoria (string): categoria del deporte.
    participante (string): nombre del participante.
    medalla (string): medalla obtenida por el participante.

Retorno:
    Sin retorno.
*/
void mover_archivo(const string& deporte, const string& categoria, const string& participante, const string& medalla){
    string directorio_base = "Carpeta Actual/" + deporte + "/" + categoria;

    crear_directorio_recursivo(directorio_base);

    string direccion_medalla;
    if((medalla == "Oro") || (medalla == "Plata") || (medalla == "Bronce")){
        direccion_medalla = directorio_base + "/ConMedalla";
        crear_directorio_recursivo(direccion_medalla); //crea el directorio de ConMedalla

        string nueva_direccion = direccion_medalla+ "/" + participante;
        if (rename((path + participante).c_str(), nueva_direccion.c_str()) != 0) { //mueve el archivo al directorio neuvo
            cerr << "Error moviendo el archivo: " << participante << " a " << nueva_direccion << endl;
        }

    } else {
        direccion_medalla = directorio_base + "/SinMedalla";
        crear_directorio_recursivo(direccion_medalla); //crea el directorio de SinMedalla

        string nueva_direccion = direccion_medalla+ "/" + participante;
        if (rename((path + participante).c_str(), nueva_direccion.c_str()) != 0) { //mueve el archivo al directorio nuevo
            cerr << "Error moviendo el archivo: " << participante << " a " << nueva_direccion << endl;
        }
    }
}


/*
crear_directorio_recursivo (void): Recorre la ruta_directorio para crear las carpetas que correspondan.
                                   Utiliza mkdir para hacer la creación, en caso de que no pueda mostrará
                                   un mensaje por consola.

Parámetros:
    ruta_directorio (string): la ruta del directorio al cual se crearán las carpetas

Retorno:
    Retorna nada solo para detener el while si es que no crea el directorio.
*/
void crear_directorio_recursivo(const string& ruta_directorio){
    size_t posicion = 0;
    string ruta_temporal = "";
    
    //buscar cada ocurrencia de / en la ruta del directorio
    while ((posicion = ruta_directorio.find('/', posicion)) != string::npos) {
        ruta_temporal = ruta_directorio.substr(0, posicion);

        if (!ruta_temporal.empty()) { //verifica que no este vacio
            errno = 0; //limpiar errno para evitar confusiones

            if (mkdir(ruta_temporal.c_str(), 0777) != 0 && errno != EEXIST) { //crea el directorio temporal si es que no existe
                cerr << "No se pudo crear el directorio: " << ruta_temporal << " Error: " << strerror(errno) << endl;
                return; //detiene la ejecucion
            }
        }

        posicion++;
    }

    errno = 0;
    if (mkdir(ruta_directorio.c_str(), 0777) != 0 && errno != EEXIST) { //termina de crear el directorio si es que no existe
        cerr << "No se pudo crear el directorio: " << ruta_directorio << " Error: " << strerror(errno) << endl;
    }
}