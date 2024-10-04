#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
using namespace std;

struct carta {
    string color;
    string tipo;
};

struct jugador {
    vector <carta> mano;
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
        mazo.push_back({"negro", "comodin"});
        mazo.push_back({"negro", "+4"});
    }
}

void revolver(vector<carta>& mazo){
    srand(time(0));
    for(size_t i = 0; i<mazo.size(); i++){
        size_t j = rand() % mazo.size();
        swap(mazo[i], mazo[j]);    
    }
}



int main(){
    vector<carta> mazo = crear_mazo();


    return 0;
}