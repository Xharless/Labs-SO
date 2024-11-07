import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.concurrent.ForkJoinPool;
import java.util.concurrent.RecursiveTask;

public class Lab3 {
    // Clase para dividir la búsqueda por cuadrantes usando ForkJoin
    static class BuscarLetraPorCuadranteTask extends RecursiveTask<Character> {
        private static final long serialVersionUID = 1L;

        private final char[][] matriz;
        private final int filaInicio, filaFin, colInicio, colFin;
        private static final int UMBRAL = 4; // Umbral para búsqueda directa

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

    // Clase para dividir la búsqueda usando hilos tradicionales
    public static class BuscarLetraTask extends Thread {
        private final char[][] matriz;
        private final int filaInicio, filaFin, colInicio, colFin;
        private static final int UMBRAL = 4;
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
            if ((filaFin - filaInicio) <= UMBRAL && (colFin - colInicio) <= UMBRAL) {
                resultado = buscarLetraDirectamente();
            } else {
                int mitadFilas = (filaInicio + filaFin) / 2;
                int mitadCols = (colInicio + colFin) / 2;

                BuscarLetraTask task1 = new BuscarLetraTask(matriz, filaInicio, mitadFilas, colInicio, mitadCols);
                BuscarLetraTask task2 = new BuscarLetraTask(matriz, filaInicio, mitadFilas, mitadCols, colFin);
                BuscarLetraTask task3 = new BuscarLetraTask(matriz, mitadFilas, filaFin, colInicio, mitadCols);
                BuscarLetraTask task4 = new BuscarLetraTask(matriz, mitadFilas, filaFin, mitadCols, colFin);

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

    public static String[] getFiles(String dirPath) {
        List<String> fileList = new ArrayList<>();
        File dir = new File(dirPath);
        if (dir.isDirectory()) {
            File[] files = dir.listFiles();
            if (files != null) {
                Arrays.sort(files, Comparator.comparingInt(file -> extractNumber(file.getName())));
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
    

    private static int extractNumber(String name) {
        String number = name.replaceAll("\\D+", "");
        return number.isEmpty() ? 0 : Integer.parseInt(number);
    }

    public static char[][] leerMatrizDeArchivo(String filePath) throws IOException {
        List<char[]> rows = new ArrayList<>();

        try (BufferedReader br = new BufferedReader(new FileReader(filePath))) {
            String linea = br.readLine();
            int size = Integer.parseInt(linea.split("x")[0].trim());

            while ((linea = br.readLine()) != null) {
                rows.add(linea.replace(" ", "").toCharArray());
            }
            return rows.toArray(new char[size][size]);
        }
    }

    public static void main(String[] args) {
        String path = "archivos_prueba/Caso1";
        String[] files = getFiles(path);

        ForkJoinPool pool = new ForkJoinPool();

        if (files != null) {
            for (String filePath : files) {
                System.out.println("Procesando archivo: " + filePath);

                try {
                    char[][] matriz = leerMatrizDeArchivo(filePath);

                    // Ejecutar con Fork/Join
                    long inicioForkJoin = System.nanoTime();
                    BuscarLetraPorCuadranteTask tareaFork = new BuscarLetraPorCuadranteTask(matriz, 0, matriz.length, 0, matriz[0].length);
                    Character letraForkJoin = pool.invoke(tareaFork);
                    long finForkJoin = System.nanoTime();
                    System.out.printf("Letra encontrada (ForkJoin) en %s: %c, Tiempo: %.3f ms\n",
                            filePath, letraForkJoin, (finForkJoin - inicioForkJoin) / 1_000_000.0);

                    // Ejecutar con hilos tradicionales
                    long inicioHilos = System.nanoTime();
                    BuscarLetraTask tareaHilos = new BuscarLetraTask(matriz, 0, matriz.length, 0, matriz[0].length);
                    tareaHilos.start();
                    tareaHilos.join();
                    Character letraHilos = tareaHilos.getResultado();
                    long finHilos = System.nanoTime();
                    System.out.printf("Letra encontrada (Hilos) en %s: %c, Tiempo: %.3f ms\n",
                            filePath, letraHilos, (finHilos - inicioHilos) / 1_000_000.0);

                    System.out.println("---------Fin del archivo---------\n");

                } catch (IOException e) {
                    System.err.println("Error al leer el archivo: " + filePath);
                    e.printStackTrace();
                } catch (InterruptedException e) {
                    System.err.println("La tarea fue interrumpida.");
                    e.printStackTrace();
                }
            }
        }
        pool.shutdown();
    }
}
