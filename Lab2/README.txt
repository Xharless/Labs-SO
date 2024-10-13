Datos personales:
	- Paralelo: 200.

	- Nombre: Daniel Maturana
    - Rol: 202173575-5

   	- Nombre: Carlos Arévalo
    - Rol: 202173501-1

Detalles de uso del programa:
	- Para ejecutar el programa, abra una terminal en el directorio donde se encuentra el main.cpp y makefile, luego escriba:
	"make run", el código se compilará y ejecutará.
	- Las opciones para jugar estarán en consola.
	- Las funcionalidades de las cartas especiales Negras tendrán un menú que se mostrará en consola.
	- Si el jugador no tiene una carta para jugar, este mismo sacaráa una carta del mazo, donde si la carta sirve se jugará automáticamente, de lo contrario
	se pasará el turno.
	- Las cartas especiales que son de tener que robar cartas, hacen que se salte el turno del jugador que debe robar cartas.
	
Consideraciones:
	- Se considera que está ejecutando el programa en un sistema Linux.
	- El código falla con respecto a los semáforos, la lógica está correcta si se ejecuta en un solo proceso, pero se bugea con la pila de descarte si
	están los demás hijos ejecuntándose, ya que no se pudo sincronizar bien utilizando semáforos.
	
Detalles de las herramientas usadas:
	- Sistema Operativo (SO): Ubuntu 22.04.4 LTS
		Release:	22.04
		Codename:	jammy