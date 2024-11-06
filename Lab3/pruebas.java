import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveTask;
import java.util.Comparator;
import java.util.Arrays;

public class pruebas {
    // Clase para dividir la búsqueda por cuadrantes
    static class BuscarLetraPorCuadranteTask extends RecursiveTask<Character> {
        private static final long serialVersionUID = 1L;

        private final char[][] matriz;
        private final int filaInicio, filaFin, colInicio, colFin;
        private static final int UMBRAL = 2; // Umbral para búsqueda directa

        public BuscarLetraPorCuadranteTask(char[][] matriz, int filaInicio, int filaFin, int colInicio, int colFin) {
            this.matriz = matriz;
            this.filaInicio = filaInicio;
            this.filaFin = filaFin;
            this.colInicio = colInicio;
            this.colFin = colFin;
        }

        @Override
        protected Character compute() {
            int numFilas = filaFin - filaInicio;
            int numCols = colFin - colInicio;

            // Caso base: si el cuadrante es pequeño, busca la letra directamente
            if (numFilas <= UMBRAL && numCols <= UMBRAL) {
                return buscarLetraDirectamente();
            } else {
                // Dividir la matriz en cuadrantes
                int mitadFilas = (filaInicio + filaFin) / 2;
                int mitadCols = (colInicio + colFin) / 2;

                BuscarLetraPorCuadranteTask task1 = new BuscarLetraPorCuadranteTask(matriz, filaInicio, mitadFilas, colInicio, mitadCols);
                BuscarLetraPorCuadranteTask task2 = new BuscarLetraPorCuadranteTask(matriz, filaInicio, mitadFilas, mitadCols, colFin);
                BuscarLetraPorCuadranteTask task3 = new BuscarLetraPorCuadranteTask(matriz, mitadFilas, filaFin, colInicio, mitadCols);
                BuscarLetraPorCuadranteTask task4 = new BuscarLetraPorCuadranteTask(matriz, mitadFilas, filaFin, mitadCols, colFin);

                // Ejecuta las tareas en paralelo y combina los resultados
                task1.fork();
                task2.fork();
                task3.fork();
                task4.fork();

                Character resultadoTask1 = task1.join();
                Character resultadoTask2 = task2.join();
                Character resultadoTask3 = task3.join();
                Character resultadoTask4 = task4.join();

                // Devolver el primer resultado distinto de '0'
                return resultadoTask1 != '0' ? resultadoTask1 :
                        resultadoTask2 != '0' ? resultadoTask2 :
                        resultadoTask3 != '0' ? resultadoTask3 : resultadoTask4;
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
    }

    // Función que lee todas las direcciones de los archivos en la carpeta
    public static String[] getFiles(String dirPath) {
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

    public static void main(String[] args) {
        String path = "archivos_prueba/Caso1"; // Cambiar el nombre del caso para leer otros
        String[] files = getFiles(path);
        
        ForkJoinPool pool = new ForkJoinPool();
        try {
            if (files != null) {
                for (String filePath : files) {
                    System.out.println("Procesando archivo: " + filePath);
                    try {
                        char[][] matriz = leerMatrizDeArchivo(filePath);
                        long inicio = System.nanoTime();
                        BuscarLetraPorCuadranteTask tarea = new BuscarLetraPorCuadranteTask(matriz, 0, matriz.length, 0, matriz[0].length);
                        Character letraEncontrada = pool.invoke(tarea);
                        
                        long fin = System.nanoTime();
                        long duracion = fin - inicio;
                        double duracionEnMilisegundos = duracion / 1_000_000.0;
                        System.out.println("Letra encontrada en " + filePath + ": " + letraEncontrada);
                        System.out.printf("Tiempo de búsqueda: %.3f milisegundos\n", duracionEnMilisegundos);
                        System.out.println("---------Fin del archivo---------\n");
                    } catch (IOException e) {
                        System.err.println("Error al leer el archivo: " + filePath);
                        e.printStackTrace();
                    }
                }
            }

        } finally {
            pool.shutdown();
        }
    }
}

