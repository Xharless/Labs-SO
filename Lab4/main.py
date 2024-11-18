import threading
import time
import random


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