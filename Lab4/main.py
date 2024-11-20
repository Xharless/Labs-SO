import os
import threading
import time
import random

LOCK_REGISTRO = threading.Lock() #Para proteger el acceso al archivo
REGISTRO = "Validacion.txt"


def validar_jugador(id_jugador,semaforo_zona):
    with semaforo_zona:

        #hora inicio
        inicio = time.strftime("%H:%M:%S")
        print(f"Jugador {id_jugador} inició validación a las {inicio}")

        #escribirlos en el registro
        with LOCK_REGISTRO:
            with open(REGISTRO, 'a') as archivo:
                archivo.write(f"{id_jugador}, {inicio}, Validación completa\n")

        #simula validacion esperando 15 segundos
        time.sleep(15)


def procesar_grupo(grupo, semaforo):
    hebras = []

    for id_jugador in grupo:
        #una hebra por jugador
        hebra = threading.Thread(target=validar_jugador, args=(id_jugador,semaforo))
        hebras.append(hebra)
        hebra.start()
    
    #espera a que todos los jugadores del grupo terminen
    for hebra in hebras:
        hebra.join()


def asignar_grupo(capacidad_grupo, num_jugadores, semaforos_zona):

    #crea los grupos
    grupos = [list(range(i, i + capacidad_grupo)) for i in range(0, num_jugadores, capacidad_grupo)]

    hebras_zonas = []
    for i, grupo in enumerate(grupos):
        semaforo = semaforos_zona[i%2] #elige semaforo por zona
        hebra_zona = threading.Thread(target=procesar_grupo, args=(grupo, semaforo))
        hebras_zonas.append(hebra_zona)
        hebra_zona.start()


#elimina si ya existe el archivo
if REGISTRO in os.listdir("."):
    os.remove(f"./{REGISTRO}")

#escribe la primera linea
with open(REGISTRO, 'w') as archivo:
    archivo.write("ID_Jugador, Hora_Inicio, Hora_Fin, Resultado\n")

capacidad_por_grupo = 32
numero_jugadores = 256
#semaforos_zona permiten que entren dos grupos
semaforos_zona = [threading.Semaphore(capacidad_por_grupo),threading.Semaphore(capacidad_por_grupo)]

asignar_grupo(capacidad_por_grupo, numero_jugadores, semaforos_zona)
archivo.close()
