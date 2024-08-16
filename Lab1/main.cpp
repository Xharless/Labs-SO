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


const string path = "./archivos_deportes/"; // los dos puntos es porque el .exe en windows se crea en la caroeta de output, entonces tiene que salir una mas arriba

void abrir_archivo();
void leer_archivo(const string&);
void crear_carpetas(const string&, const string&, const string& , const string&);
void crear_directorio(const string&);

int main(){
    abrir_archivo();


    return 0;
}


void abrir_archivo(){
    if(DIR* pDIR = opendir(path.c_str())){ 
        cout << "DIRECTORIO ENCONTRADO " << endl;
        while(dirent* entry = readdir(pDIR)){
            string fileName = entry->d_name;  
            if(fileName != "." && fileName != ".."){  
                cout<<fileName<<endl;
                leer_archivo(fileName);
            }
        }  
        closedir(pDIR);
    } else {
        cout << "NO SE PUEDE ABRIR EL DIRECTORIO" << endl; 
    }
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

        crear_carpetas(deporte, categoria, fileName, medalla);

    } else {
        cout << "NO SE PUDO ABRIR EL ARCHIVO" << endl;
    }
}

void crear_carpetas(const string& deporte, const string& categoria, const string& participante , const string& medalla){
    string baseD = "Carpeta Actual/" + deporte + "/" + categoria;
    crear_directorio("Carpeta Actual");
    crear_directorio(baseD);

    string medallaD;
    if((medalla == "Oro") || (medalla == "Plata") || (medalla == "Bronce")){
        medallaD = baseD + "/Con Medalla";
        crear_directorio(medallaD);
        string newfilepath = medallaD + "/" + participante;
        if (rename((path + "/" + participante).c_str(), newfilepath.c_str()) != 0) {
            cerr << "Error moviendo el archivo: " << participante << " a " << newfilepath << endl;
        }

    } else {
        medallaD = baseD + "/Sin Medalla";
        crear_directorio(medallaD);
        string newfilepath = medallaD + "/" + participante;
        if (rename((path + "/" + participante).c_str(), newfilepath.c_str()) != 0) {
            cerr << "Error moviendo el archivo: " << participante << " a " << newfilepath << endl;
        }
    }
    
}

void crear_directorio(const string& dirPath ){
    struct stat info;
    if (stat(dirPath.c_str(), &info) != 0){
        #if defined(_WIN32) || defined(_WIN64)
        if (_mkdir(dirPath.c_str()) != 0) {
        #else
        if (mkdir(dirPath.c_str(), 0777) != 0) {
        #endif
            cerr << "No se pudo crear el directorio: " << dirPath << endl;
        }
    }
}