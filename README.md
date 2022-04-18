# Sistema de estacionamiento

El ejercicio es _simular un sistema de estacionamiento_ con las siguientes características:

- Capacidad de estacionamiento 15 autos
  - Solamente puede haber entre 0 - 15 autos en el estacionaminto
  - Hay un led verde, que indica que hay espacio en el estacionamiento.
  - Hay un led rojo, que indica que el estacionamiento está lleno, y no deja pasar a nadie más.
- Sistema de sensor de tarjeta y peso para entrar y salir pasar los caros
  - Se representan como 2 botones. Uno para la tarjeta, y otro para el peso.
  - Un led que representa la pluma (opcionalmente puede ser un motor)
  - Para que el auto pueda entrar o salir, se tiene que seguir la siguiente secuencia:
    1. Activar el sensor del peso.
    2. Activar el sensor de la tarjeta.
    3. Desactivar el sensor de la tarjeta
    4. Desactivar el sensor del peso
  - Después de la secuencia, se considera que el auto salió o entró al estacionamiento, y se actualiza la cuenta.
- Display con la cantidad de autos en el estacionamiento

El proyecto se estará haciendo con _arduino_.
