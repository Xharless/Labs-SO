import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException; 
import java.io.FileReader; 
import java.io.IOException; 
import java.util.List;
import java.util.ArrayList;

public class pruebas {
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
        String path = "archivos_prueba/Caso1";
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
    }
}
