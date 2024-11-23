import os
import threading
import time
import random

LOCK_REGISTRO = threading.Lock()  #protege acceso al archivo
CONDICION_FASE = threading.Condition() #protege la ronda
REGISTRO_VALIDACION = "Validacion.txt"
REGISTRO_FINAL = "Final.txt"

#configuracion del torneo
NUMERO_JUGADORES = 256
CAPACIDAD_POR_GRUPO = 32
DURACION_VALIDACION = 15
DURACION_ENFRENTAMIENTO = 10
GANADOR = []
REPECHAJE = []
TERMINADO = True


def registrar_resultado(nombre_archivo, contenido):
    with LOCK_REGISTRO:
        with open(nombre_archivo, "a") as archivo:
            archivo.write(contenido + "\n")


def validar_jugador(id_jugador, semaforo_zona):
    with semaforo_zona:
        #hora inicio
        inicio = time.strftime("%H:%M:%S")
        print(f"Jugador {id_jugador} inició validación a las {inicio}")

        #escribirlos en el registro
        resultado = "Validacion Completa"
        registrar_resultado(REGISTRO_VALIDACION, f"{id_jugador}, {inicio}, {resultado}")

        #simula validacion esperando 15 segundos
        time.sleep(DURACION_VALIDACION)


def procesar_grupo(grupo, semaforo):
    hebras = []

    for id_jugador in grupo:
        #una hebra por jugador
        hebra = threading.Thread(target=validar_jugador, args=(id_jugador, semaforo))
        hebras.append(hebra)
        hebra.start()

    #espera a que todos los jugadores del grupo terminen
    for hebra in hebras:
        hebra.join()


def asignar_grupo(capacidad_grupo, num_jugadores, semaforos_zona):
    #crea los grupos
    grupos = [list(range(i, i + capacidad_grupo)) for i in range(0, num_jugadores, capacidad_grupo)]

    for i, grupo in enumerate(grupos):
        semaforo = semaforos_zona[i % 2]  #elige semaforo por zona
        hebra_zona = threading.Thread(target=procesar_grupo, args=(grupo, semaforo))
        hebra_zona.start()


def enfrentar_jugadores(jugador1, jugador2):
    print(f"Enfrentando a {jugador1} y {jugador2}...")
    ganador = random.choice([jugador1, jugador2])
    print(f"Ganador: {ganador}")
    return ganador


def fase_eliminacion_por_ronda(jugadores, ronda):
    #inicializa archivo
    archivo_ganadores = f'Ganadores_Ronda{ronda}.txt'
    with open(archivo_ganadores, "w") as archivo:
        archivo.write("Enfrentamiento -> Ganador\n")

    ganadores = []
    perdedores = []
    for i in range(0, len(jugadores), 2):
        ganador = enfrentar_jugadores(jugadores[i], jugadores[i + 1])
        ganadores.append(ganador)
        perdedores.append(jugadores[i] if ganador != jugadores[i] else jugadores[i + 1])

        registrar_resultado(archivo_ganadores, f"{jugadores[i]} vs {jugadores[i + 1]} -> {ganador}")

    time.sleep(DURACION_ENFRENTAMIENTO)
    return ganadores, perdedores


def fase_repechaje_por_ronda(losers, ronda):
    #inicializa archivo
    archivo_repechaje = f"Perdedores_Ronda{ronda}.txt"
    with open(archivo_repechaje, "w") as archivo:
        archivo.write("Enfrentamiento -> Ganador\n")

    ganadores = []
    perdedores = []
    for i in range(0, len(losers), 2):
        ganador = enfrentar_jugadores(losers[i], losers[i + 1])
        ganadores.append(ganador)
        perdedores.append(losers[i] if ganador != losers[i] else losers[i + 1])

        registrar_resultado(archivo_repechaje, f"{losers[i]} vs {losers[i + 1]} -> {ganador}")

    #se eliminan los perdedores
    time.sleep(DURACION_ENFRENTAMIENTO)
    return ganadores


def fase_eliminacion(ganadores_eliminacion):
    global GANADOR, TERMINADO, REPECHAJE

    with CONDICION_FASE:
        ronda = 2
        cant_jugadores = len(ganadores_eliminacion)
        ganadores_ronda_eliminacion = ganadores_eliminacion

        while cant_jugadores > 1:
            while not TERMINADO:
                CONDICION_FASE.wait()

            #comienza la ronda
            TERMINADO = True
            print(f"Comienza la ronda {ronda} con {cant_jugadores} jugadores en eliminación")
            ganadores, perdedores = fase_eliminacion_por_ronda(ganadores_ronda_eliminacion, ronda)
            ganadores_ronda_eliminacion = ganadores  #los que ganan van a la siguiente ronda
            REPECHAJE += perdedores

            #terminamos la ronda
            ronda += 1
            cant_jugadores -= len(ganadores)
            TERMINADO = False
            CONDICION_FASE.notify_all()

        GANADOR = ganadores_ronda_eliminacion
        print(f"Termina fase de eliminación con {ronda-1} rondas")


def fase_repechaje():
    global REPECHAJE, TERMINADO, GANADOR

    with CONDICION_FASE:
        ronda_repechaje = 2
        cant_jugadores = len(REPECHAJE)

        while cant_jugadores > 1:
            while TERMINADO:
                CONDICION_FASE.wait()

            #comienza la ronda
            TERMINADO = False
            print(f"Comienza la ronda {ronda_repechaje} de perdedores con {cant_jugadores} jugadores")
            ganadores = fase_repechaje_por_ronda(REPECHAJE, ronda_repechaje)

            REPECHAJE = ganadores

            #termina la ronda
            ronda_repechaje += 1
            cant_jugadores = len(REPECHAJE)

            #si existe ganador entonces debe seguir iterando
            if GANADOR:
                TERMINADO = False
            else:
                TERMINADO = True
            CONDICION_FASE.notify_all()
        print(f"Termina fase de repechaje con {ronda_repechaje-1} rondas")


#elimina archivos en caso de que existan
if REGISTRO_VALIDACION in os.listdir("."):
    os.remove(f"./{REGISTRO_VALIDACION}")
if REGISTRO_FINAL in os.listdir("."):
    os.remove(f"./{REGISTRO_FINAL}")

#escribe formato de validacion
with open(REGISTRO_VALIDACION, 'w') as archivo_validacion:
    archivo_validacion.write("ID_Jugador, Hora_Inicio, Resultado\n")

#semaforos para dos grupos
semaforos_zona = [threading.Semaphore(CAPACIDAD_POR_GRUPO), threading.Semaphore(CAPACIDAD_POR_GRUPO)]

#se validan jugadores por grupo
asignar_grupo(CAPACIDAD_POR_GRUPO, NUMERO_JUGADORES, semaforos_zona)
jugadores_validos = [f"Jugador{i}" for i in range(1, NUMERO_JUGADORES + 1)]

#juegan la primera ronda y se crean ambos torneos
ganadores_eliminacion, perdedores_eliminacion = fase_eliminacion_por_ronda(jugadores_validos, 1)
ganadores_repechaje = fase_repechaje_por_ronda(perdedores_eliminacion, 1)
REPECHAJE += ganadores_repechaje

#fase eliminacion activa siempre y cuando termina la ronda manda señal a la de repechaje para que continue con la cantidad sumada de jugadores
hebras_fases = []
hebra_fase_eliminacion = threading.Thread(target=fase_eliminacion, args=(ganadores_eliminacion,))
hebras_fases.append(hebra_fase_eliminacion)
hebra_fase_eliminacion.start()

hebra_fase_repechaje = threading.Thread(target=fase_repechaje)
hebras_fases.append(hebra_fase_repechaje)
hebra_fase_repechaje.start()

for hebra in hebras_fases:
    hebra.join()


#ronda final
finalista_eliminacion = GANADOR[0]
finalista_repechaje = REPECHAJE[0]
ganador_final = enfrentar_jugadores(finalista_eliminacion, finalista_repechaje)

with open(REGISTRO_FINAL, "w") as archivo_final:
    archivo_final.write("Enfrentamiento Final -> Ganador\n")
    archivo_final.write(f"{finalista_eliminacion} vs {finalista_repechaje} -> {ganador_final}\n")
