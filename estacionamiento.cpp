/*
 * name: Eduardo Gomez
 * mat: 550775
 *
 * Este programa simula un sistema de estacionamiento. Cuenta cuantos carros hay
 * en el estacionamiento y con esa informacion, despliega si el estacionamiento
 * esta lleno o vacio.
 */

#include <LiquidCrystal.h>

using bit = char; // Bit para poder guardar el estado de un objeto

// Pins for connecting
#define LED_DISPONIBLE 10
#define LED_LLENO 9
#define PLUMA_ENTRADA 8
#define PLUMA_SALIDA 13
#define SENSOR_PESO_ENTRADA 0
#define SENSOR_PESO_SALIDA 6
#define SENSOR_TARJETA_ENTRADA 1
#define SENSOR_TARJETA_SALIDA 7
#define CAPACIDAD_ESTACIONAMIENTO 15

/***********Configuracion del LCD******************/
// Esta configuracion es recomendada por Arduino
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
/**************************************************/

/*
 * CounterwithLimit
 *
 * It keeps track of a count, in which it has a lower bound and an upper bound
 * [l,u). It also shows if you are on a limit.
 */
class CounterWithLimit
{

private:
    int lowerBound;
    int upperBound;
    int count;

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
        return this->count == this->upperBound - 1;
    }

    int increment()
    {
        if (!this->isOnUpperLimit())
        {
            this->count++;
        }
        return this->count;
    }

    int decrement()
    {
        if (!this->isOnLowerLimit())
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

/*
 * This is a wrapper to handle button reads
 */
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

    /*
     * void read()
     * Reads the value of the pin, and updates the state of the button
     */
    void read()
    {
        this->state = digitalRead(this->pin);
    }

    /*
     * int getState()
     * Gets the state of the button, doesn't read again.
     */
    int getState()
    {
        return this->state;
    }
};

/*
 *  Led
 * It's a Wrapper to handle the Leds
 */
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

/*
 *  ParkingPenSystem
 * It handles the ParkingPen System, including updating it's internal state and handling the pen
 */
class ParkingPenSystem
{

private:
    Button weightSensor;
    Button cardSensor;
    Led pluma;

public:
    // These are the Possible states of the Pen
    enum SystemStates
    {
        CLOSED_PEN,   // Iddle, with the pen closed.
        CAR_DETECTED, // Weight sensor is active
        OPENED_PEN,   // Both weight sensor and card sensor are active
        PASSING_CAR,  // After opening the pen, when the car leaves, then it sends a pulse for closing the pen.
    };

    ParkingPenSystem() {}

    ParkingPenSystem(int pinPeso, int pinTarjeta, int pinPluma)
    {
        this->weightSensor = Button(pinPeso);
        this->cardSensor = Button(pinTarjeta);
        this->pluma = Led(pinPluma);
    }

private:
    SystemStates state = CLOSED_PEN;

    void updateState()
    {
        // Reads the sensors
        this->weightSensor.read();
        this->cardSensor.read();

        switch (this->state)
        {

        case CLOSED_PEN:
            if (this->weightSensor.getState() == HIGH)
            {
                this->state = CAR_DETECTED;
            }
            break;

        case CAR_DETECTED:

            // The car went off without using the card sensor
            if (this->weightSensor.getState() == LOW)
            {
                this->state = CLOSED_PEN;
            }

            // The car puts the card in the car sensor
            else if (this->cardSensor.getState() == HIGH)
            {
                this->state = OPENED_PEN;
            }

            // If none of the previous conditions are met, it means the car has not moved yet.
            break;

        case OPENED_PEN:

            //  When the car leaves, then it must close the pen, regardless of the card state.
            if (this->weightSensor.getState() == LOW)
            {
                this->state = PASSING_CAR; // Manda un pulso de que se esta cerrando la pluma
            }
            // If it has not moved, the pen must remain opened
            break;

        case PASSING_CAR:
            // Because it is a pulse, then it automatically closes and goes into the waiting state
            this->state = CLOSED_PEN;
            break;
        }
    }

    void handlePluma()
    {
        this->state == OPENED_PEN ? this->pluma.on() : this->pluma.off();
    }

public:
    SystemStates getState()
    {
        return this->state;
    }

    // Updates the state of the system, based on the inputs and the state
    void update()
    {
        this->updateState();
        this->handlePluma();
    }

    // Manually open
    void open()
    {
        this->state = OPENED_PEN;
        this->handlePluma();
    }

    // Manually closes
    void close()
    {
        this->state = CLOSED_PEN;
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

    enum SistemaPlumasIdx
    {
        ENTRANCE,
        EXIT
    };

public:
    Parking(int capacity, LiquidCrystal *logger, ParkingPenSystem entrancePenSystem, ParkingPenSystem exitPenSystem, int availableLedPin, int fullLedPin)
    {
        this->carCounter = CounterWithLimit(0, capacity + 1, 0);
        this->logger = logger;
        this->parkingPen[ENTRANCE] = entrancePenSystem;
        this->parkingPen[EXIT] = exitPenSystem;
        this->availableLed = Led(availableLedPin);
        this->fullLed = Led(fullLedPin);
    }

private:
    void updateState()
    {
        this->state = this->carCounter.isOnLowerLimit() ? EMPTY : this->carCounter.isOnUpperLimit() ? FULL
                                                                                                    : AVAILABLE;
    }

    // Checks if the pen sistem has a closing pen state,
    bool didACarPassThePen(ParkingPenSystem &penSystem)
    {
        return ParkingPenSystem::PASSING_CAR == penSystem.getState();
    }

    void handleOutputs()
    {

        switch (this->state)
        {
        case FULL:
            // this->logger->setCursor(0, 2);
            // this->logger->print("full");
            this->parkingPen[ENTRANCE].close();
            this->parkingPen[EXIT].update();
            this->availableLed.off();
            this->fullLed.on();
            break;

        case EMPTY:

            // this->logger->setCursor(0, 2);
            // this->logger->print("empty");
            this->parkingPen[ENTRANCE].update();
            this->parkingPen[EXIT].close();
            this->availableLed.on();
            this->fullLed.off();
            break;

        case AVAILABLE:
            // this->logger->setCursor(0, 2);
            // this->logger->print("dis");
            this->parkingPen[ENTRANCE].update();
            this->parkingPen[EXIT].update();
            this->availableLed.on();
            this->fullLed.off();
            break;
        }

        if (this->didACarPassThePen(this->parkingPen[ENTRANCE]))
        {
            this->carCounter.increment();
        }

        if (this->didACarPassThePen(this->parkingPen[EXIT]))
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

Parking parking(CAPACIDAD_ESTACIONAMIENTO, &lcd,
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