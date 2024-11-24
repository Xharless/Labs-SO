Detalles de uso del programa:
	- Nombre del archivo principal "main.py". Este debe encontrarse en la carpeta en la que desee que queden los archivos .txt.
	- Para ejecutar, abrir una terminal en la carpeta del archivo principal y ejecutar "python3 main.py". Asegúrese tener la
	  versión de Python 3 o superior.
	- Una vez terminada la ejecución encontrará los archivos .txt de los ganadores de las rondas fase eliminación y
	  repechaje, además de la validación y final. Estos tendrán nombre como: "Validacion.txt", "Ganadores_Ronda1.txt",
	  "Perdedores_Ronda1.txt", "Final.txt", el número de la ronda va a ir cambiando crecientemente.

Consideraciones:
	- Se asume que siempre serán 256 jugadores en el torneo.
	- Los jugadores que pierden en fase de eliminación se sumarán a la ronda actual de repechaje.
	- El primer lugar de la fase de eliminación va a esperar que terminen los de la fase de repechaje para enfrentar al ganador.
	- En la terminal se mostrarán la situación actual del torneo, sin embargo esto es solo para fines de debugging.
	- Se debe esperar a que se generen todos los archivos .txt para poder visualizar su contenido.

Detalles de las herramientas usadas:
	- Sistema Operativo: Ubuntu 24.04.1 LTS (GNU/Linux 5.15.153.1-microsoft-standard-WSL2 x86_64)
	- Lenguaje: Python 3.12.3
