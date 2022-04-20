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
    bool isOnLimit()
    {
        return this->count == this->upperBound || this->count == this->lowerBound;
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

    int read()
    {
        this->state = digitalRead(this->pin);
        return this->state;
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
    Led pinPluma;

    enum EstadoPluma
    {
        ESPERANDO,
        DETECTA_PESO,
        SCAN_TARJETA,     // Solo pasa si se esta detectando un peso
        QUITANDO_TARJETA, // Solo pasa si se quita la tarjeta despues del peso
        SALIENDO_PESO,    // Despues de quitar la tarjeta
    };

    EstadoPluma estado = ESPERANDO;

public:
    SistemaPluma() {}

    SistemaPluma(int pinPeso, int pinTarjeta, int pinPluma)
    {
        this->sensorPeso = Button(pinPeso);
        this->sensorTarjeta = Button(pinTarjeta);
        this->pinPluma = Led(pinPluma);
    }
};

class Estacionamiento
{

private:
    CounterWithLimit counterCarros;
    LiquidCrystal *logger;

    enum SalidasIndice
    {
        LLENO,
        DISPONIBLE
    };

    enum SistemaPlumasIndices
    {
        ENTRADA,
        SALIDA
    };

    SistemaPluma sistemasPluma[2];

public:
    Estacionamiento(int capacidad, LiquidCrystal *logger, SistemaPluma plumaEntrada, SistemaPluma plumaSalida)
    {
        this->counterCarros = CounterWithLimit(0, capacidad + 1, 0);
        this->logger = logger;
        this->sistemasPluma[ENTRADA] = plumaEntrada;
        this->sistemasPluma[SALIDA] = plumaSalida;
    }

    void carIn()
    {
        this->counterCarros.increment();
    }

    void carOut()
    {
        this->counterCarros.decrement();
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
                                      SistemaPluma(SENSOR_PESO_SALIDA, SENSOR_TARJETA_SALIDA, PLUMA_SALIDA));

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