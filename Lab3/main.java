/*
compila: javac main.java
ejecuta: java main
*/
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveTask;
import java.io.BufferedReader; 
import java.io.FileNotFoundException; 
import java.io.FileReader; 
import java.io.IOException; 


//Ejemplo de hilos
class MiHilo extends Thread {
    private String nombre;

    public MiHilo(String nombre) {
        this.nombre = nombre;
    }

    @Override
    public void run() {
        for (int i = 1; i <= 3; i++) {
            System.out.println(nombre + " ejecutando iteración " + i);
            try {
                Thread.sleep(500); // Pausa de medio segundo (500 ms)
            } catch (InterruptedException e) {
                System.out.println(nombre + " fue interrumpido");
            }
        }
        System.out.println(nombre + " ha terminado su ejecución.");
    }
}


public class main {
    // funcion para leer lo que contiene los archivos
    public static void muestraContenido(String archivo) throws FileNotFoundException, IOException{
        String cadena;
        FileReader f = new FileReader(archivo);
        BufferedReader b = new BufferedReader(f);
        while ((cadena = b.readLine()) != null) {
            System.out.println(cadena);
        }
        b.close();
    }
    public static void main(String[] args) {
        muestraContenido("Lab3/archivos_prueba/Caso1/code1.txt");
        MiHilo hilo1 = new MiHilo("Hilo 1");
        MiHilo hilo2 = new MiHilo("Hilo 2");

        hilo1.start(); // Iniciar Hilo 1
        hilo2.start(); // Iniciar Hilo 2
    }
}
