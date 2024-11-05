
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
    // La clase que hara que se busque la letra en la matriz
    static class BuscarLetraPorCuadranteTask extends RecursiveTask<Character> {
        private final char[][] matriz;
        private final char letra;
        private final int filaInicio;
        private final int filaFin;
        private final int colInicio;
        private final int colFin;
        private static final int UMBRAL = 4; // Umbral para búsqueda directa

        public BuscarLetraPorCuadranteTask(char[][] matriz, char letra, int filaInicio, int filaFin, int colInicio, int colFin){
            this.matriz = matriz;
            this.letra = letra;
            this.filaInicio = filaInicio;
            this.filaFin = filaFin;
            this.colInicio = colInicio;
            this.colFin = colFin;
        }

        @Override
        protected Character compute(){
            int numFilas = filaFin - filaInicio;
            int numCols = colFin - colInicio;
            System.out.println("Computando cuadrante: [" + filaInicio + ", " + filaFin + "] x [" + colInicio + ", " + colFin + "]");

            // caso base, si el cuadrante es pequeño, busca la letra directamente
            if (numFilas <= UMBRAL && numCols <= UMBRAL){
                System.out.println("Entrando a buscar directamente en cuadrante.");
                return buscarLetraDirectamente();
            } else {
                // Dividir la matriz en cuadrantes
                int mitadFilas = (filaInicio + filaFin) / 2;
                int mitadCols = (colInicio + colFin) / 2;
                BuscarLetraPorCuadranteTask task1 = new BuscarLetraPorCuadranteTask(matriz, letra, filaInicio, mitadFilas, colInicio, mitadCols);
                BuscarLetraPorCuadranteTask task2 = new BuscarLetraPorCuadranteTask(matriz, letra, filaInicio, mitadFilas, mitadCols, colFin); // Cuadrante superior derecho
                BuscarLetraPorCuadranteTask task3 = new BuscarLetraPorCuadranteTask(matriz, letra, mitadFilas, filaFin, colInicio, mitadCols); // Cuadrante inferior izquierdo
                BuscarLetraPorCuadranteTask task4 = new BuscarLetraPorCuadranteTask(matriz, letra, mitadFilas, filaFin, mitadCols, colFin); // Cuadrante inferior derecho

                // ahora ejecuta las tareas en paralelo y combina los resultados
                task1.fork();
                task2.fork();
                task3.fork();
                task4.fork();

                Character resultadoTask1 = task1.join();
                Character resultadoTask2 = task2.join();
                Character resultadoTask3 = task3.join();
                Character resultadoTask4 = task4.join();
                
                // Devolver el primer resultado no 0 que se encuentre
                return resultadoTask1 != null ? resultadoTask1 :
                        resultadoTask2 != null ? resultadoTask2 :
                        resultadoTask3 != null ? resultadoTask3 :
                        resultadoTask4;
            }
        }
        private Character buscarLetraDirectamente() {
            System.out.println("hola");
            for (int i = filaInicio; i < filaFin; i++) {
                for (int j = colInicio; j < colFin; j++) {
                    System.out.println("Carácter encontrado en la matriz:");
                    if (matriz[i][j] != '0') {
                        System.out.println("Carácter encontrado en la matriz: " + matriz[i][j]); // Imprimir el carácter encontrado
                        return matriz[i][j]; // Retornar el primer carácter distinto de '0'
                    }
                }
            }
            return null; // Si no se encontró un carácter diferente de '0'
        }
    }
    
    
    
    // funcion que lee todas las direcciones de la carpeta (en este caso es Caso1)
    public static String[] getFiles(String dir_path) throws IOException {
        List<String> res = new ArrayList<>();
        File dir = new File(dir_path);
        if (dir.isDirectory()) {
            File[] arr_content = dir.listFiles();
            if (arr_content != null) { 
                for (File file : arr_content) {
                    if (file.isFile()) {
                        res.add(file.getAbsolutePath()); 
                    }
                }
            }
        } else {
            System.err.println("Path no válido");
        }
        return res.toArray(new String[0]); 
    }

    public static void main(String[] args) throws IOException {
        String path = "archivos_prueba/Caso1"; // aqui hay que cambiar el nombre del caso para que lea el siguiebte
        try {
            String[] files = getFiles(path);
            for (String filePath : files) {
                System.out.println("Leyendo archivo: " + filePath);
                try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
                    String linea;
                    while ((linea = br.readLine()) != null) {
                        System.out.println(linea);
                    }
                } catch (IOException e) {
                    System.err.println("Error al leer el archivo: " + filePath);
                    e.printStackTrace();
                }
                System.out.println("---------Fin del archivo---------\n");
            }
        } catch (IOException e) {
            System.err.println("Error al obtener archivos del directorio: " + e.getMessage());
        }
        MiHilo hilo1 = new MiHilo("Hilo 1");
        MiHilo hilo2 = new MiHilo("Hilo 2");

        hilo1.start(); // Iniciar Hilo 1
        hilo2.start(); // Iniciar Hilo 2 
    }
}
