/*
compila: javac main.java
         javac -Xlint:all -Werror pruebas.java
ejecuta: java main
*/

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException; 
import java.io.FileReader; 
import java.io.IOException; 
import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;

public class main {
    //Ejemplo de hilos
    public static class BuscarLetraTask extends Thread {
        private final char[][] matriz;
        private final int filaInicio, filaFin, colInicio, colFin;
        private static final int UMBRAL = 4; // Umbral para búsqueda directa
        private Character resultado = '0';

        public BuscarLetraTask(char[][] matriz, int filaInicio, int filaFin, int colInicio, int colFin) {
            this.matriz = matriz;
            this.filaInicio = filaInicio;
            this.filaFin = filaFin;
            this.colInicio = colInicio;
            this.colFin = colFin;
        }

        @Override
        public void run() {
            // Si el cuadrante es pequeño, realiza la búsqueda directamente
            if ((filaFin - filaInicio) <= UMBRAL && (colFin - colInicio) <= UMBRAL) {
                resultado = buscarLetraDirectamente();
            } else {
                // Dividir la matriz en cuadrantes
                int mitadFilas = (filaInicio + filaFin) / 2;
                int mitadCols = (colInicio + colFin) / 2;

                // Crear subhilos para cada cuadrante
                BuscarLetraTask task1 = new BuscarLetraTask(matriz, filaInicio, mitadFilas, colInicio, mitadCols);
                BuscarLetraTask task2 = new BuscarLetraTask(matriz, filaInicio, mitadFilas, mitadCols, colFin);
                BuscarLetraTask task3 = new BuscarLetraTask(matriz, mitadFilas, filaFin, colInicio, mitadCols);
                BuscarLetraTask task4 = new BuscarLetraTask(matriz, mitadFilas, filaFin, mitadCols, colFin);

                // Ejecutar y unir los hilos de forma sincrónica
                task1.start();
                task2.start();
                task3.start();
                task4.start();

                try {
                    task1.join();
                    task2.join();
                    task3.join();
                    task4.join();

                } catch (InterruptedException e) {
                    Thread.currentThread().interrupt();
                }

                // Obtener el primer resultado distinto de '0'
                resultado = task1.getResultado() != '0' ? task1.getResultado() :
                            task2.getResultado() != '0' ? task2.getResultado() :
                            task3.getResultado() != '0' ? task3.getResultado() :
                            task4.getResultado();
            }
        }

        private Character buscarLetraDirectamente() {
            for (int i = filaInicio; i < filaFin; i++) {
                for (int j = colInicio; j < colFin; j++) {
                    if (matriz[i][j] != '0') {
                        return matriz[i][j];
                    }
                }
            }
            return '0';
        }

        public Character getResultado() {
            return resultado;
        }
    }

    // Función para leer el contenido de un archivo y convertirlo en una matriz de caracteres
    public static char[][] leerMatrizDeArchivo(String filePath) throws IOException {
        List<char[]> rows = new ArrayList<>();

        try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
            String linea = br.readLine();  // Leer la primera línea para conocer el tamaño
            int size = Integer.parseInt(linea.split("x")[0].trim());

            while ((linea = br.readLine()) != null) {
                rows.add(linea.replace(" ", "").toCharArray());
            }
            
            // Convertir la lista de filas a una matriz cuadrada
            return rows.toArray(new char[size][size]);
        }
    }

    // funcion que lee todas las direcciones de la carpeta (en este caso es Caso1)
    public static String[] getFiles(String dirPath) throws FileNotFoundException, IOException{
        List<String> fileList = new ArrayList<>();
        File dir = new File(dirPath);

        if (dir.isDirectory()) {
            File[] files = dir.listFiles();
            if (files != null) {
                Arrays.sort(files, new Comparator<File>() {
                    @Override
                    public int compare(File f1, File f2) {
                        // Extraer el número del nombre del archivo y comparar numéricamente
                        int num1 = extractNumber(f1.getName());
                        int num2 = extractNumber(f2.getName());
                        return Integer.compare(num1, num2);
                    }
    
                    private int extractNumber(String name) {
                        // Suponiendo que el nombre del archivo tiene formato "codeX"
                        String number = name.replaceAll("\\D+", ""); // Remueve cualquier cosa que no sea dígito
                        return number.isEmpty() ? 0 : Integer.parseInt(number);
                    }
                });
                for (File file : files) {
                    if (file.isFile()) {
                        fileList.add(file.getAbsolutePath());
                    }
                }
            }
        } else {
            System.err.println("Path no válido");
        }
        return fileList.toArray(new String[0]);
    }

    public static void main(String[] args) throws IOException {
        String path = "archivos_prueba/Caso1"; // aqui hay que cambiar el nombre del caso para que lea el siguiebte
        String[] files = getFiles(path);

        if(files != null ){
            for(String filePath : files){
                System.out.println("Procesando archivo: " + filePath);

                try {
                    char[][] matriz = leerMatrizDeArchivo(filePath);
                    long inicio = System.nanoTime();

                    BuscarLetraTask tarea = new BuscarLetraTask(matriz, 0, matriz.length, 0, matriz[0].length);
                    tarea.start();

                    tarea.join();
                    Character letraEncontrada = tarea.getResultado();

                    long fin = System.nanoTime();
                    long duracion = fin - inicio;
                    double duracionEnMilisegundos = duracion / 1_000_000.0;

                    System.out.println("Letra encontrada en " + filePath + ": " + letraEncontrada);
                    System.out.printf("Tiempo de búsqueda: %.3f milisegundos\n", duracionEnMilisegundos);
                    System.out.println("---------Fin del archivo---------\n");

                } catch (IOException e){
                    System.err.println("Error al leer el archivo: " + filePath );
                    e.printStackTrace();
                } catch (InterruptedException e) {
                    System.err.println("La tarea fue interrumpida.");
                    e.printStackTrace();
                }
            }
        }
    }
}
