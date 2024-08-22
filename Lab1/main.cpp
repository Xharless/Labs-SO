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

const string path = "./archivos_deportes/";
//const string path = "/home/cam/Escritorio/archivos_deportes/";

//Definicion de funciones
void lista_archivos();
void leer_archivo(const string&);
void mover_archivo(const string&, const string&, const string&, const string&);
void crear_directorio_recursivo(const string&);


int main(){
    lista_archivos();
    return 0;
}

//string path: la ruta pero en string
//DIR *direccion: la ruta pero en tipo DIR
void lista_archivos(){
    string nombre_archivo;
    DIR *direccion;

    //verifica que puede abrir el directorio y lee cada archivo dentro de este
    if(DIR* direccion = opendir(path.c_str())){ 
        while(dirent *entry = readdir(direccion)){
            nombre_archivo = entry->d_name;

            //excluye la carpeta actual (.) y la anterior (..) 
            if(nombre_archivo != "." && nombre_archivo != ".."){  
                cout << "Leyendo archivo: " << nombre_archivo << endl;
                leer_archivo(nombre_archivo);
            }
        }  

    } else {
        cout << "NO SE PUEDE ABRIR EL DIRECTORIO" << endl; 
    }

    closedir(direccion);
}

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

        cout << "Archivo leído exitosamente: " << nombre_archivo << endl;
        cout << "Deporte: " << deporte << endl;
        cout << "Categoría: " << categoria << endl;
        cout << "Medalla: " << medalla << endl;

        mover_archivo(deporte, categoria, nombre_archivo, medalla);

    } else {
        cout << "NO SE PUDO ABRIR EL ARCHIVO: " << direccion_archivo << endl;
    }

    file.close();
}

void mover_archivo(const string& deporte, const string& categoria, const string& participante, const string& medalla){
    string directorio_base = "Carpeta Actual/" + deporte + "/" + categoria;
    cout << "Creando directorio base: " << directorio_base << endl;

    crear_directorio_recursivo(directorio_base);

    string direccion_medalla;
    if((medalla == "Oro") || (medalla == "Plata") || (medalla == "Bronce")){
        direccion_medalla = directorio_base + "/ConMedalla";
        crear_directorio_recursivo(direccion_medalla); //crea el directorio de ConMedalla

        string nueva_direccion = direccion_medalla+ "/" + participante;
        if (rename((path + participante).c_str(), nueva_direccion.c_str()) != 0) { //mueve el archivo al directorio neuvo
            cerr << "Error moviendo el archivo: " << participante << " a " << nueva_direccion << endl;
        } else {
            cout << "Archivo movido a: " << nueva_direccion << endl;
        }

    } else {
        direccion_medalla = directorio_base + "/SinMedalla";
        crear_directorio_recursivo(direccion_medalla); //crea el directorio de SinMedalla

        string nueva_direccion = direccion_medalla+ "/" + participante;
        if (rename((path + participante).c_str(), nueva_direccion.c_str()) != 0) { //mueve el archivo al directorio nuevo
            cerr << "Error moviendo el archivo: " << participante << " a " << nueva_direccion << endl;
        } else {
            cout << "Archivo movido a: " << nueva_direccion << endl;
        }
    }
}

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
    } else {
        cout << "Directorio creado: " << ruta_directorio << endl;
    }
}
