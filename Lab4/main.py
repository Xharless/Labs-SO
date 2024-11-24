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


'''
registrar_resultado: Abre el archivo correspondiente y escribe como contenido un texto específico.

Parámetros:
    nombre_archivo (string): nombre del archivo a escribir
    contenido (string): texto que se escribe en el archivo.

Retorno:
    Sin retorno.
'''
def registrar_resultado(nombre_archivo, contenido):
    with LOCK_REGISTRO:
        with open(nombre_archivo, "a") as archivo:
            archivo.write(contenido + "\n")


'''
validar_jugador: Inicia la validación, contando su hora de inicio y llamando a la función
                 registrar_resultado para escribir en el archivo. Simula la espera de 15 segundos.

Parámetros:
    id_jugador (int): número identificador del jugador.
    semaforo_zona (threading.Semaphore): semáforo para proteger el archivo mientras se escribe.

Retorno:
    Sin retorno.
'''
def validar_jugador(id_jugador, semaforo_zona):
    with semaforo_zona:
        #hora inicio
        inicio = time.strftime("%H:%M:%S")
        print(f"Jugador {id_jugador} inició validación a las {inicio}")

        #escribirlos en el registro
        resultado = "Validación Completa"
        registrar_resultado(REGISTRO_VALIDACION, f"Jugador {id_jugador}, {inicio}, {resultado}")

        #simula validacion esperando 15 segundos
        time.sleep(DURACION_VALIDACION)


'''
procesar_grupo: Recorre la lista del grupo y le asigna una hebra por jugador, así entra cada uno
                a validarse.

Parámetros:
    grupo (lista int): lista que contiene los id de jugadores.
    semaforo (threading.Semaphore): semáforo para proteger el archivo mientras se escribe.

Retorno:
    Sin retorno.
'''
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


'''
asignar_grupo: Asigna los 256 jugadores a grupos con capacidad máxima de 32 jugadores. Para las
               zonas se tienen hebras, pero se van compartiendo los dos semáforos así van de
               dos en dos.

Parámetros:
    capacidad_grupo (int): cantidad máxima de jugadores por grupo.
    num_jugadores (int): cantidad de jugadores total del torneo.
    semaforos_zona (lista threading.Semaphore): contiene dos semáforos para cada zona.

Retorno:
    Sin retorno.
'''
def asignar_grupo(capacidad_grupo, num_jugadores, semaforos_zona):
    #crea los grupos
    grupos = [list(range(i, i + capacidad_grupo)) for i in range(0, num_jugadores, capacidad_grupo)]

    for i, grupo in enumerate(grupos):
        semaforo = semaforos_zona[i % 2]  #elige semaforo por zona
        hebra_zona = threading.Thread(target=procesar_grupo, args=(grupo, semaforo))
        hebra_zona.start()


'''
enfrentar_jugadores: Se escoge aleatoriamente a un jugador de los dos y se declara ganador.
                     Simula un enfrentamiento 1vs1.

Parámetros:
    jugador1 (str): nombre jugador 1.
    jugador2 (str): nombre jugador 2.

Retorno:
    ganador (str): jugador escogido por random choice.
'''
def enfrentar_jugadores(jugador1, jugador2):
    print(f"Enfrentando a {jugador1} y {jugador2}...")
    ganador = random.choice([jugador1, jugador2])
    print(f"Ganador: {ganador}")
    return ganador


'''
fase_eliminacion_por_ronda: Se crea el archivo de la ronda y se recorre la lista de
                            jugadores para hacer enfrentamientos entre parejas, se registran
                            los ganadores, luego se retornan los ganadores y perdedores.
                            Simula la espera de 10 segundos.

Parámetros:
    jugadores (lista str): lista de los jugadores que van a enfrentarse.
    ronda (int): ronda actual de la fase de eliminación.

Retorno:
    ganadores (lista str): lista de jugadores que ganaron en esta ronda.
    perdedores (lista str): lista de jugadores que perdieron en esta ronda.
'''
def fase_eliminacion_por_ronda(jugadores, ronda):
    #inicializa archivo
    archivo_ganadores = f'Ganadores_Ronda{ronda}.txt'
    with open(archivo_ganadores, "w") as archivo:
        archivo.write("Enfrentamiento, Hora_Enfrentamiento -> Ganador\n")

    ganadores = []
    perdedores = []
    for i in range(0, len(jugadores), 2):
        hora_enfrentamiento = time.strftime("%H:%M:%S")
        ganador = enfrentar_jugadores(jugadores[i], jugadores[i + 1])
        ganadores.append(ganador)
        perdedores.append(jugadores[i] if ganador != jugadores[i] else jugadores[i + 1])

        registrar_resultado(archivo_ganadores, f"{jugadores[i]} vs {jugadores[i + 1]}, {hora_enfrentamiento} -> {ganador}")

    time.sleep(DURACION_ENFRENTAMIENTO)
    return ganadores, perdedores


'''
fase_repechaje_por_ronda: Se crea el archivo de la ronda y se recorre la lista de
                          jugadores para hacer enfrentamientos entre parejas, se registran
                          los ganadores para luego retornarlos. Simula la espera de 10 segundos.

Parámetros:
    losers (lista str): lista de los jugadores que perdieron en eliminación.
    ronda (int): ronda actual de la fase de repechaje.

Retorno:
    ganadores (lista str): lista de jugadores que ganaron en esta ronda.
'''
def fase_repechaje_por_ronda(losers, ronda):
    #inicializa archivo
    archivo_repechaje = f"Perdedores_Ronda{ronda}.txt"
    with open(archivo_repechaje, "w") as archivo:
        archivo.write("Enfrentamiento, Hora_Enfrentamiento -> Ganador\n")

    ganadores = []
    for i in range(0, len(losers), 2):
        hora_enfrentamiento = time.strftime("%H:%M:%S")
        ganador = enfrentar_jugadores(losers[i], losers[i + 1])
        ganadores.append(ganador)

        registrar_resultado(archivo_repechaje, f"{losers[i]} vs {losers[i + 1]}, {hora_enfrentamiento} -> {ganador}")

    #se eliminan los perdedores
    time.sleep(DURACION_ENFRENTAMIENTO)
    return ganadores


'''
fase_eliminacion: Comienza el ciclo con la ronda 2, determina si la ronda de repechaje ha terminado para seguir
                  o se queda esperando la señal, suma a la variable global REPECHAJE los perdedores de la ronda y
                  actualiza la lista de ganadores_ronda_eliminacion, una vez actualiza los datos de la ronda manda
                  la señal a la fase_repechaje para que continúe. Cuando terminan las rondas actualiza la variable
                  global GANADOR al último jugador. 

Parámetros:
    ganadores_eliminacion (lista str): lista de los jugadores que ganaron la ronda anterior.

Retorno:
    Sin retorno.
'''
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


'''
fase_repechaje: Comienza el ciclo con la ronda 2, determina si la ronda de eliminación ha terminado para seguir
                o se queda esperando la señal, actualiza la variable global REPECHAJE los ganadores de la ronda y
                una vez actualiza los datos de la ronda manda la señal a la fase_eliminacion para que continúe.
                Si terminan las rondas de eliminación verifica la variable global GANADOR que sea vacía para esperar
                a la fase_eliminacion, si contiene algo entonces no sigue esperando e itera hasta el final para actualizar
                a REPECHAJE con el último ganador. 

Parámetros:
    Sin parámetros.

Retorno:
    Sin retorno.
'''
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
    archivo_validacion.write("Jugador, Hora_Inicio, Resultado\n")

#semaforos para dos grupos
semaforos_zona = [threading.Semaphore(CAPACIDAD_POR_GRUPO), threading.Semaphore(CAPACIDAD_POR_GRUPO)]

#se validan jugadores por grupo
asignar_grupo(CAPACIDAD_POR_GRUPO, NUMERO_JUGADORES, semaforos_zona)
jugadores_validos = [f"Jugador{i}" for i in range(1, NUMERO_JUGADORES + 1)]

#juegan la primera ronda y se crean ambos torneos
ganadores_eliminacion, perdedores_eliminacion = fase_eliminacion_por_ronda(jugadores_validos, 1)
ganadores_repechaje = fase_repechaje_por_ronda(perdedores_eliminacion, 1)
REPECHAJE += ganadores_repechaje

#fase eliminacion activa y cuando termina la ronda manda señal a la de repechaje para que continue con la cantidad sumada de jugadores
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
hora_final = time.strftime("%H:%M:%S")
ganador_final = enfrentar_jugadores(finalista_eliminacion, finalista_repechaje)

with open(REGISTRO_FINAL, "w") as archivo_final:
    archivo_final.write("Enfrentamiento Final, Hora_Enfrentamiento -> Ganador\n")
    archivo_final.write(f"{finalista_eliminacion} vs {finalista_repechaje}, {hora_final} -> {ganador_final}\n")
