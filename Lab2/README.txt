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
	- Si el jugador no tiene una carta para jugar, este mismo sacará una carta del mazo, donde si la carta sirve se jugará automáticamente, de lo contrario
	se pasará el turno.
	- Las cartas especiales que son de tener que robar cartas, hacen que se salte el turno del jugador que debe robar cartas.
	- Cuando se gana, se muestra la cantidad de cartas de los otros jugadores. 
	- Si al momento de escoger color con las cartas especiales se sale del rango de la selección, se tomara el color Amarillo como default.
	- Si escogemos una carta no valida, o colocamos una opción no valida, se mostrará un mensaje de error, haciendo que volvamos a escoger una opción valida.
	
Consideraciones:
	- Se considera que está ejecutando el programa en un sistema Linux.
	- El código falla con respecto a los semáforos, la lógica está correcta si se ejecuta en un solo proceso, pero se bugea con la pila de descarte si
	están los demás hijos ejecuntándose, ya que no se pudo sincronizar bien utilizando semáforos.
	- Cuando se termina la partida, se muestra la cantidad de cartas de los demas jugadores. Pero, al parecer, hay procesos que se siguen ejecutando de una forma extraña, lo que 
	hace que se muestren en pantalla ciertos mensajes de jugabilidad y luego finaliza la partida.
Detalles de las herramientas usadas:
	- Sistema Operativo (SO): Ubuntu 22.04.4 LTS
		Release:	22.04
		Codename:	jammy