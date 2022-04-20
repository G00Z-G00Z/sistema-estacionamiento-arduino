#include <LiquidCrystal.h>
#define LED_DISPONIBLE 10
#define LED_LLENO 9
#define PLUMA_ENTRADA 8
#define PLUMA_SALIDA 13
#define SENSOR_PESO_ENTRADA 0
#define SENSOR_PESO_SALIDA 1
#define SENSOR_TARJETA_ENTRADA 6
#define SENSOR_TARJETA_SALIDA 7
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
        pinMode(pin, OUTPUT);
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

class ParkingPenSystem
{

private:
    Button weightSensor;
    Button cardSensor;
    Led pluma;

public:
    // Con esto detectamos si la pluma esta abierta o cerrada
    enum SystemStates
    {
        WAITING,
        CAR_DETECTED,
        OPENED_PEN,
        CLOSING_PEN,
    };

private:
    SystemStates state = WAITING;

    void updateState()
    {
        // Lectura de sensores
        this->weightSensor.read();
        this->cardSensor.read();

        // Update del estado
        switch (this->state)
        {
        // No ha pasado nada
        case WAITING:
            if (this->weightSensor.getState() == HIGH)
            {
                this->state = CAR_DETECTED;
            }
            break;

        // Ahora espera el scan de la tarjeta, o que se quite
        case CAR_DETECTED:

            // Se quito el carro
            if (this->weightSensor.getState() == LOW)
            {
                this->state = WAITING;
            }
            else
                // El carro esta, y se pone la tarjeta
                if (this->cardSensor.getState() == HIGH)
                {
                    this->state = OPENED_PEN;
                }

            // Si no se cumple ninguna de las condiciones, el estado se deja igual
            break;

        case OPENED_PEN:
            // Se quita el carro de la pesa, el auto ya paso.
            if (this->weightSensor.getState() == LOW)
            {
                this->state = CLOSING_PEN; // Manda un pulso de que se esta cerrando la pluma
            }

            break;

        case CLOSING_PEN: // Representa el pulso para poder contar
            this->state = WAITING;
            break;

        default:
            this->state = WAITING;
        }
    }

    void handlePluma()
    {
        this->state == OPENED_PEN ? this->pluma.on() : this->pluma.off();
    }

public:
    ParkingPenSystem() {}

    ParkingPenSystem(int pinPeso, int pinTarjeta, int pinPluma)
    {
        this->weightSensor = Button(pinPeso);
        this->cardSensor = Button(pinTarjeta);
        this->pluma = Led(pinPluma);
    }

    SystemStates getState()
    {
        return this->state;
    }

    void update()
    {
        this->updateState();
        this->handlePluma();
    }

    void open()
    {
        this->state = OPENED_PEN;
        this->handlePluma();
    }

    void close()
    {
        this->state = WAITING;
        this->handlePluma();
    }
};

class Parking
{

private:
    CounterWithLimit carCounter;
    LiquidCrystal *logger;
    ParkingPenSystem parkingPen[2];
    Led availableLed;
    Led fullLed;

    enum ParkingStates
    {
        FULL,
        EMPTY,
        AVAILABLE,
    };

    ParkingStates state = AVAILABLE;

    enum SistemaPlumasIndices
    {
        ENTRANCE,
        EXIT
    };

public:
    Parking(int capacidad, LiquidCrystal *logger, ParkingPenSystem plumaEntrada, ParkingPenSystem plumaSalida, int pinLedDisponible, int pinLedLleno)
    {
        this->carCounter = CounterWithLimit(0, capacidad + 1, 0);
        this->logger = logger;
        this->parkingPen[ENTRANCE] = plumaEntrada;
        this->parkingPen[EXIT] = plumaSalida;
        this->availableLed = Led(pinLedDisponible);
        this->fullLed = Led(pinLedLleno);
    }

private:
    void updateState()
    {
        this->state = this->carCounter.isOnLowerLimit() ? EMPTY : this->carCounter.isOnUpperLimit() ? FULL
                                                                                                    : AVAILABLE;
    }

    bool didACarPassThePluma(ParkingPenSystem &sistemaPluma)
    {
        return ParkingPenSystem::CLOSING_PEN == sistemaPluma.getState();
    }

    void handleOutputs()
    {

        switch (this->state)
        {
        case FULL:
            this->parkingPen[ENTRANCE].close();
            this->parkingPen[EXIT].update();
            this->availableLed.off();
            this->fullLed.on();
            break;

        case EMPTY:
            this->parkingPen[ENTRANCE].update();
            this->parkingPen[EXIT].close();
            this->availableLed.on();
            this->fullLed.off();
            break;

        case AVAILABLE:
            this->parkingPen[ENTRANCE].update();
            this->parkingPen[EXIT].update();
            this->availableLed.on();
            this->fullLed.off();
            break;
        }

        if (this->didACarPassThePluma(this->parkingPen[ENTRANCE]))
        {
            this->carCounter.increment();
        }

        if (this->didACarPassThePluma(this->parkingPen[EXIT]))
        {
            this->carCounter.decrement();
        }

        // Imprime la cuenta
        this->logger->setCursor(0, 1);
        this->logger->print(this->carCounter.getCount());
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

Parking parking(15, &lcd,
                ParkingPenSystem(SENSOR_PESO_ENTRADA, SENSOR_TARJETA_ENTRADA, PLUMA_ENTRADA),
                ParkingPenSystem(SENSOR_PESO_SALIDA, SENSOR_TARJETA_SALIDA, PLUMA_SALIDA), LED_DISPONIBLE, LED_LLENO);

void setup()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
}

void loop()
{
    parking.update();
}