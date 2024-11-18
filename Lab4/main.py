import threading
import time
import random

capacidad_por_zona = 32 
semaforo_zona = threading.Semaphore(capacidad_por_zona)


lock_registro = threading.Lock() #Para proteger el acceso al archivo
registro = "Validacion.txt"


def validar_jugador(id_jugador):
    with semaforo_zona:
        inicio = time.strftime("%H:%M:%S") #hora inicio que lo piden en el archivo final
        print(f"Jugador {id_jugador} iniciio validacion a las {inicio}")
        time.sleep(random.randint(1,3))

        resultado = "Validacion completa" if random.random() > 0.2 else "Validacion fallida"
        fin = time.strftime("%H:%M:%S")
        print(f"Jugador {id_jugador} termina validacion a las {fin} con resultado: {resultado}")

        #escribirlos en el registro
        with lock_registro:
            with open(registro, 'a') as archivo:
                archivo.write(f"{id_jugador}, {inicio}, {resultado}\n")



# comportamiento de un jugador
def jugador(id_jugador):
    print(f"Jugador {id_jugador} entrando al torneo")
    time.sleep(random.randint(1, 3)) # simular el tiempo de validacion
    print(f"Jugador {id_jugador} completo la validacion")

def crear_jugadores(num_jugadores):
    hebras = []
    for i in range(num_jugadores):
        #aqui crea una hebra por cada jugador
        hebra = threading.Thread(target = jugador, args=(i,))
        hebras.append(hebra)
        hebra.start() #para iniciar la hebra

        
    # para ver que todas las hebras terminen antes de seguir
    for hebra in hebras:
        hebra.join()

crear_jugadores(256)
