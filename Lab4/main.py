import os
import threading
import time
import random

LOCK_REGISTRO = threading.Lock()  # Para proteger el acceso al archivo
REGISTRO_VALIDACION = "Validacion.txt"
REGISTRO_FINAL = "Final.txt"

# Configuración del torneo
NUMERO_JUGADORES = 256
CAPACIDAD_POR_GRUPO = 32
DURACION_VALIDACION = 15
DURACION_ENFRENTAMIENTO = 10


def registrar_resultado(nombre_archivo, contenido):
    with LOCK_REGISTRO:
        with open(nombre_archivo, "a") as archivo:
            archivo.write(contenido + "\n")


def validar_jugador(id_jugador, semaforo_zona):
    with semaforo_zona:
        # hora inicio
        inicio = time.strftime("%H:%M:%S")
        print(f"Jugador {id_jugador} inició validación a las {inicio}")

        # escribirlos en el registro
        resultado = "Validacion Completa"
        registrar_resultado(REGISTRO_VALIDACION, f"{id_jugador}, {inicio}, {resultado}")

        # simula validacion esperando 15 segundos
        time.sleep(DURACION_VALIDACION)


def procesar_grupo(grupo, semaforo):
    hebras = []

    for id_jugador in grupo:
        # una hebra por jugador
        hebra = threading.Thread(target=validar_jugador, args=(id_jugador, semaforo))
        hebras.append(hebra)
        hebra.start()

    # espera a que todos los jugadores del grupo terminen
    for hebra in hebras:
        hebra.join()


def asignar_grupo(capacidad_grupo, num_jugadores, semaforos_zona):
    # crea los grupos
    grupos = [list(range(i, i + capacidad_grupo)) for i in range(0, num_jugadores, capacidad_grupo)]

    hebras_zonas = []
    for i, grupo in enumerate(grupos):
        semaforo = semaforos_zona[i % 2]  # elige semaforo por zona
        hebra_zona = threading.Thread(target=procesar_grupo, args=(grupo, semaforo))
        hebras_zonas.append(hebra_zona)
        hebra_zona.start()


def enfrentar_jugadores(jugador1, jugador2):
    print(f"Enfrentando a {jugador1} y {jugador2}...")
    ganador = random.choice([jugador1, jugador2])
    print(f"Ganador: {ganador}")
    return ganador


def fase_eliminacion(jugadores, ronda):
    # Inicializa archivo para los ganadores de la ronda
    archivo_ganadores = f'Ganadores_Ronda{ronda}.txt'
    # Sobrescribe el archivo si ya existe
    with open(archivo_ganadores, "w") as archivo:
        archivo.write("Enfrentamiento -> Ganador\n")

    ganadores = []
    perdedores = []  # Lista para los perdedores de esta ronda
    for i in range(0, len(jugadores), 2):
        ganador = enfrentar_jugadores(jugadores[i], jugadores[i + 1])
        ganadores.append(ganador)
        perdedores.append(jugadores[i] if ganador != jugadores[i] else jugadores[i + 1])  # El perdedor de esta ronda

        registrar_resultado(archivo_ganadores, f"{jugadores[i]} vs {jugadores[i + 1]} -> {ganador}")

    # Guardar los perdedores en un archivo
    archivo_perdedores = f"Perdedores_Ronda{ronda}.txt"
    with open(archivo_perdedores, "w") as archivo:
        archivo.write("Enfrentamiento -> Perdedor\n")
        for jugador in perdedores:
            archivo.write(f"{jugador}\n")

    time.sleep(DURACION_ENFRENTAMIENTO)
    return ganadores, perdedores  # Regresamos también los perdedores


def fase_repechaje(losers, ronda):
    archivo_repechaje = f"Perdedores_Ronda{ronda}.txt"
    with open(archivo_repechaje, "w") as archivo:
        archivo.write("Enfrentamiento -> Ganador\n")

    ganadores_repechaje = []
    i = 0
    while i < len(losers) - 1:  # Procesa de dos en dos mientras haya suficientes jugadores
        ganador = enfrentar_jugadores(losers[i], losers[i + 1])
        ganadores_repechaje.append(ganador)
        registrar_resultado(archivo_repechaje, f"{losers[i]} vs {losers[i + 1]} -> {ganador}")
        i += 2

    # Si queda un jugador sin pareja, pasa automáticamente a la siguiente ronda
    if len(losers) % 2 != 0:
        print(f"{losers[-1]} pasa automáticamente a la siguiente ronda.")
        ganadores_repechaje.append(losers[-1])
        registrar_resultado(archivo_repechaje, f"{losers[-1]} -> Pasa automáticamente")

    time.sleep(DURACION_ENFRENTAMIENTO)
    return ganadores_repechaje


# Elimina si ya existen los archivos
if REGISTRO_VALIDACION in os.listdir("."):
    os.remove(f"./{REGISTRO_VALIDACION}")
if REGISTRO_FINAL in os.listdir("."):
    os.remove(f"./{REGISTRO_FINAL}")

# Escribe primera línea como formato
with open(REGISTRO_VALIDACION, 'w') as archivo_validacion:
    archivo_validacion.write("ID_Jugador, Hora_Inicio, Resultado\n")

# Semáforos_zona permiten que entren dos grupos
semaforos_zona = [threading.Semaphore(CAPACIDAD_POR_GRUPO), threading.Semaphore(CAPACIDAD_POR_GRUPO)]

# Validación inicial
asignar_grupo(CAPACIDAD_POR_GRUPO, NUMERO_JUGADORES, semaforos_zona)
jugadores_validos = [f"Jugador{i}" for i in range(1, NUMERO_JUGADORES + 1)]

# Fase de eliminación directa
ganadores_ronda = jugadores_validos
ronda = 1
cant_jugadores = len(jugadores_validos)
while cant_jugadores > 1:
    print(f"Comienza la ronda {ronda} con {cant_jugadores} jugadores")
    ganadores_eliminacion, perdedores_eliminacion = fase_eliminacion(ganadores_ronda, ronda)
    ganadores_ronda = ganadores_eliminacion  # Los que avanzan a la siguiente ronda
    ronda += 1
    cant_jugadores -= len(ganadores_eliminacion)
print(f"Termina fase de eliminación con {ronda} rondas")

# Fase de repechaje (solo los perdedores)
losers_ronda = perdedores_eliminacion  # Solo los perdedores
ronda_repechaje = 1
cant_perdedores = len(losers_ronda)
while cant_perdedores > 1:
    print(f"Comienza la ronda {ronda_repechaje} de perdedores con {cant_perdedores} jugadores")
    ganadores_repechaje = fase_repechaje(losers_ronda, ronda_repechaje)
    losers_ronda = ganadores_repechaje  # Los que avanzan en el repechaje
    ronda_repechaje += 1
    cant_perdedores -= len(ganadores_repechaje)
print(f"Termina fase de repechaje con {ronda_repechaje} rondas")

# Ronda final
finalista_eliminacion = ganadores_ronda
finalista_repechaje = losers_ronda
ganador_final = enfrentar_jugadores(finalista_eliminacion, finalista_repechaje)

with open(REGISTRO_FINAL, "w") as archivo_final:
    archivo_final.write("Enfrentamiento Final -> Ganador\n")
    archivo_final.write(f"{finalista_eliminacion} vs {finalista_repechaje} -> {ganador_final}\n")
