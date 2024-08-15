#include <stdio.h>
#include <string>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;


void abrir_archivo();


int main(){
    abrir_archivo();
    

    return 0;
}


void abrir_archivo(){
    if(DIR* pDIR = opendir("../archivos_deportes")){ // los dos puntos es porque el .exe en windows se crea en la caroeta de output, entonces tiene que salir una mas arriba
        while(dirent* entry = readdir(pDIR)){
            string fileName = entry->d_name;  
            if(fileName != "." && fileName != ".."){  
                cout << fileName << endl;  
            }
        }  
        closedir(pDIR);
    } else {
        cout << "No se puede abrir el directorio" << endl; 
    }
}