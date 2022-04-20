#include <LiquidCrystal.h>

/*
 * CounterwithLimit
 *
 * It keeps track of a count, in which it has a lower bound and an upper bound
 * [l,u). It also shows if you are on a limit.
 */

using bit = char;

class CounterWithLimit
{

private:
    int lowerBound;
    int upperBound;
    int count = 0;

public:
    CounterWithLimit()
    {
        this->lowerBound = 0;
        this->upperBound = 0;
        this->count = 0;
    }

    CounterWithLimit(int lowerBound, int upperBound, int initialValue)
    {
        this->lowerBound = lowerBound;
        this->upperBound = upperBound;
        this->count = initialValue;
    }

public:
    int getCount()
    {
        return this->count;
    }

    bool isOnLowerLimit()
    {
        return this->count == this->lowerBound;
    }

    bool isOnUpperLimit()
    {
        return this->count == this->upperBound;
    }

    int increment()
    {
        if (this->upperBound > 1 + this->count)
        {
            this->count++;
        }
        return this->count;
    }

    int decrement()
    {
        if (this->lowerBound <= 1 + this->count)
        {
            this->count--;
        }
        return this->count;
    }

    // Setters y getters

    void setUpperLimit(int upperLimit)
    {
        this->upperBound = upperLimit;
    }

    int getUpperLimit()
    {
        return this->upperBound;
    }

    void setLowerLimit(int lowerLimit)
    {
        this->lowerBound = lowerLimit;
    }

    int getLowerLimit()
    {
        return this->lowerBound;
    }
};

class Button
{

private:
    bit state = LOW;
    int pin;

public:
    Button()
    {
        this->pin = 0;
    }

    Button(int pin)
    {
        this->pin = pin;
        pinMode(pin, INPUT);
    }

    void read()
    {
        this->state = digitalRead(this->pin);
    }

    int getState()
    {
        return this->state;
    }
};

class Led
{

private:
    int pin;

public:
    Led()
    {
        this->pin = 0;
    }

    Led(int pin)
    {
        this->pin = pin;
        pinMode(pin, INPUT);
    }

    void on()
    {
        digitalWrite(this->pin, HIGH);
    }

    void off()
    {
        digitalWrite(this->pin, LOW);
    }
};

class SistemaPluma
{

private:
    Button sensorPeso;
    Button sensorTarjeta;
    Led pluma;

public:
    // Con esto detectamos si la pluma esta abierta o cerrada
    enum EstadoPluma
    {
        ESPERANDO,
        DETECTAR_CARRO,
        PLUMA_ABIERTA,
        CERRANDO_PLUMA,
    };

private:
    EstadoPluma estado = ESPERANDO;

    void updateState()
    {

        // Lectura de sensores
        this->sensorPeso.read();
        this->sensorTarjeta.read();

        // Update del estado
        switch (this->estado)
        {
        // No ha pasado nada
        case ESPERANDO:
            if (this->sensorPeso.getState() == HIGH)
            {
                this->estado = DETECTAR_CARRO;
            }
            break;

        // Ahora espera el scan de la tarjeta, o que se quite
        case DETECTAR_CARRO:

            // Se quito el carro
            if (this->sensorPeso.getState() == LOW)
            {
                this->estado = ESPERANDO;
            }
            else
                // El carro esta, y se pone la tarjeta
                if (this->sensorTarjeta.getState() == HIGH)
                {
                    this->estado = PLUMA_ABIERTA;
                }

            // Si no se cumple ninguna de las condiciones, el estado se deja igual
            break;

        case PLUMA_ABIERTA:
            // Se quita el carro de la pesa, el auto ya paso.
            if (this->sensorPeso.getState() == LOW)
            {
                this->estado = CERRANDO_PLUMA; // Manda un pulso de que se esta cerrando la pluma
            }

            break;

        case CERRANDO_PLUMA: // Representa el pulso para poder contar
            this->estado = ESPERANDO;
            break;

        default:
            this->estado = ESPERANDO;
        }
    }

    void handlePluma()
    {
        this->estado == PLUMA_ABIERTA ? this->pluma.on() : this->pluma.off();
    }

public:
    SistemaPluma() {}

    SistemaPluma(int pinPeso, int pinTarjeta, int pinPluma)
    {
        this->sensorPeso = Button(pinPeso);
        this->sensorTarjeta = Button(pinTarjeta);
        this->pluma = Led(pinPluma);
    }

    EstadoPluma getState()
    {
        return this->estado;
    }

    void update()
    {
        this->updateState();
        this->handlePluma();
    }

    void open()
    {
        this->estado = PLUMA_ABIERTA;
        this->handlePluma();
    }

    void close()
    {
        this->estado = ESPERANDO;
        this->handlePluma();
    }
};

class Estacionamiento
{

private:
    CounterWithLimit counterCarros;
    LiquidCrystal *logger;
    SistemaPluma sistemasPluma[2];
    Led ledDisponible;
    Led ledLleno;

    enum EstadoEstacionamiento
    {
        LLENO,
        VACIO,
        DISPONIBLE,
    };

    EstadoEstacionamiento state = DISPONIBLE;

    enum SistemaPlumasIndices
    {
        ENTRADA,
        SALIDA
    };

public:
    Estacionamiento(int capacidad, LiquidCrystal *logger, SistemaPluma plumaEntrada, SistemaPluma plumaSalida, int pinLedDisponible, int pinLedLleno)
    {
        this->counterCarros = CounterWithLimit(0, capacidad + 1, 0);
        this->logger = logger;
        this->sistemasPluma[ENTRADA] = plumaEntrada;
        this->sistemasPluma[SALIDA] = plumaSalida;
        this->ledDisponible = Led(pinLedDisponible);
        this->ledLleno = Led(pinLedLleno);
    }

private:
    void updateState()
    {
        this->state = this->counterCarros.isOnLowerLimit() ? VACIO : this->counterCarros.isOnUpperLimit() ? LLENO
                                                                                                          : DISPONIBLE;
    }

    bool didACarPassThePluma(SistemaPluma &sistemaPluma)
    {
        return SistemaPluma::CERRANDO_PLUMA == sistemaPluma.getState();
    }

    void handleOutputs()
    {

        switch (this->state)
        {
        case LLENO:
            this->sistemasPluma[ENTRADA].close();
            this->sistemasPluma[SALIDA].update();
            this->ledDisponible.off();
            this->ledLleno.on();
            break;

        case VACIO:
            this->sistemasPluma[ENTRADA].update();
            this->sistemasPluma[SALIDA].close();
            this->ledDisponible.on();
            this->ledLleno.off();
            break;

        case DISPONIBLE:
            this->sistemasPluma[ENTRADA].update();
            this->sistemasPluma[SALIDA].update();
            this->ledDisponible.on();
            this->ledLleno.off();
            break;
        }

        if (this->didACarPassThePluma(this->sistemasPluma[ENTRADA]))
        {
            this->counterCarros.increment();
        }

        if (this->didACarPassThePluma(this->sistemasPluma[SALIDA]))
        {
            this->counterCarros.decrement();
        }

        // Imprime la cuenta
        lcd.print(this->counterCarros.getCount());
    }

public:
    void update()
    {
        this->updateState();
        this->handleOutputs();
    }
};

/***********Configuracion del LCD******************/
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
/**************************************************/

#define SENSOR_PESO_ENTRADA 0
#define SENSOR_PESO_SALIDA 1
#define SENSOR_TARJETA_ENTRADA 6
#define SENSOR_TARJETA_SALIDA 7
#define PLUMA_ENTRADA 8
#define PLUMA_SALIDA 13

const Estacionamiento estacionamiento(15, &lcd,
                                      SistemaPluma(SENSOR_PESO_ENTRADA, SENSOR_TARJETA_ENTRADA, PLUMA_ENTRADA),
                                      SistemaPluma(SENSOR_PESO_SALIDA, SENSOR_TARJETA_SALIDA, PLUMA_SALIDA), 0, 0);

void setup()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    // Print a message to the LCD.
    lcd.print("Sistema de las plumas");
}

void loop()
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
}