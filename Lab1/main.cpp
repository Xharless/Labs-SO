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

//Definicion de funciones
void lista_archivos();
void leer_archivo(const string&);
void crear_carpetas(const string&, const string&, const string&, const string&);
void crear_directorio_recursivo(const string&);


int main(){
    lista_archivos();
    return 0;
}

//string path: la ruta pero en string
//DIR *direccion: la ruta pero en tipo DIR
void lista_archivos(){
    string path, nombre_archivo;
    DIR *direccion;
    
    path = "./archivos_deportes/";

    //verifica que puede abrir el directorio y lee cada archivo dentro de este
    if(DIR* direccion = opendir(path.c_str())){ 
        while(dirent *entry = readdir(direccion)){
            nombre_archivo = entry->d_name;

            //excluye la carpeta actual (.) y la anterior (..) 
            if(nombre_archivo != "." && nombre_archivo != ".."){  
                cout << "Leyendo archivo: " << nombre_archivo << endl;
                //leer_archivo(nombre_archivo);
            }
        }  

    } else {
        cout << "NO SE PUEDE ABRIR EL DIRECTORIO" << endl; 
    }

    closedir(direccion);
}

void leer_archivo(const string& fileName){
    string filePath = path + fileName;
    ifstream file(filePath);
    if (file.is_open() ){
        string deporte, categoria, medalla;
        getline(file, deporte);
        getline(file, categoria);
        getline(file, medalla);
        file.close();

        cout << "Archivo leído exitosamente: " << fileName << endl;
        cout << "Deporte: " << deporte << ", Categoría: " << categoria << ", Medalla: " << medalla << endl;

        crear_carpetas(deporte, categoria, fileName, medalla);

    } else {
        cout << "NO SE PUDO ABRIR EL ARCHIVO: " << filePath << endl;
    }
}

void crear_carpetas(const string& deporte, const string& categoria, const string& participante, const string& medalla){
    string baseD = "Carpeta Actual/" + deporte + "/" + categoria;
    cout << "Creando directorio base: " << baseD << endl;

    crear_directorio_recursivo(baseD);

    string medallaD;
    if((medalla == "Oro") || (medalla == "Plata") || (medalla == "Bronce")){
        medallaD = baseD + "/Con Medalla";
        crear_directorio_recursivo(medallaD);
        string newfilepath = medallaD + "/" + participante;
        if (rename((path + participante).c_str(), newfilepath.c_str()) != 0) {
            cerr << "Error moviendo el archivo: " << participante << " a " << newfilepath << endl;
        } else {
            cout << "Archivo movido a: " << newfilepath << endl;
        }
    } else {
        medallaD = baseD + "/Sin Medalla";
        crear_directorio_recursivo(medallaD);
        string newfilepath = medallaD + "/" + participante;
        if (rename((path + participante).c_str(), newfilepath.c_str()) != 0) {
            cerr << "Error moviendo el archivo: " << participante << " a " << newfilepath << endl;
        } else {
            cout << "Archivo movido a: " << newfilepath << endl;
        }
    }
}

void crear_directorio_recursivo(const string& dirPath ){
    size_t pos = 0;
    string tempPath = "";
    
    while ((pos = dirPath.find('/', pos)) != string::npos) {
        tempPath = dirPath.substr(0, pos);
        if (!tempPath.empty()) {
            #if defined(_WIN32) || defined(_WIN64)
            if (_mkdir(tempPath.c_str()) != 0 && errno != EEXIST) {
            #else
            if (mkdir(tempPath.c_str(), 0777) != 0 && errno != EEXIST) {
            #endif
                cerr << "No se pudo crear el directorio: " << tempPath << endl;
                return;
            }
        }
        pos++;
    }

    #if defined(_WIN32) || defined(_WIN64)
    if (_mkdir(dirPath.c_str()) != 0 && errno != EEXIST) {
    #else
    if (mkdir(dirPath.c_str(), 0777) != 0 && errno != EEXIST) {
    #endif
        cerr << "No se pudo crear el directorio: " << dirPath << endl;
    } else {
        cout << "Directorio creado: " << dirPath << endl;
    }
}
