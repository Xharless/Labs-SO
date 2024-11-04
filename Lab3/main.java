/*
compila: javac main.java
ejecuta: java main
*/

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException; 
import java.io.FileReader; 
import java.io.IOException; 
import java.util.List;
import java.util.ArrayList;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveTask;

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
    // funcion que lee todas las direcciones de la carpeta (en este caso es Caso1)
    public static String[] getFiles(String dir_path) throws FileNotFoundException, IOException{
        String[] arr_res = null;
        File f = new File(dir_path);
        if(f.isDirectory()){
            List<String> res = new ArrayList<>();
            File[] arr_content = f.listFiles();
            int size = arr_content.length;
            for(int i = 0; i<size; i++){
                if(arr_content[i].isFile())
                res.add(arr_content[i].toString());
            }
            arr_res = res.toArray(new String[0]);
        } else {
            System.err.println("Path no valido");
        }
        return arr_res;
    }

    public static void main(String[] args) throws IOException {
        String path = "archivos_prueba/Caso1"; // aqui hay que cambiar el nombre del caso para que lea el siguiebte
        String[] files = getFiles(path);
        if(files != null ){
            for(String filePath : files){
                System.out.println("Leyendo archivo: " + filePath);
                try(BufferedReader br = new BufferedReader(new FileReader(filePath))){
                    String linea;
                    while((linea = br.readLine()) != null){
                        System.out.println(linea);
                    }
                } catch (IOException e){
                    System.err.println("Error al leer el archivo: " + filePath );
                    e.printStackTrace();
                }
                System.out.println("---------Fin del archivo---------\n");
            }
        }
        MiHilo hilo1 = new MiHilo("Hilo 1");
        MiHilo hilo2 = new MiHilo("Hilo 2");

        hilo1.start(); // Iniciar Hilo 1
        hilo2.start(); // Iniciar Hilo 2
    }
}
