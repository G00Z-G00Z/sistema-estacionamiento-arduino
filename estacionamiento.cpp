#include <LiquidCrystal.h>

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

class Estacionamiento
{

private:
    CounterWithLimit counterCarros;
    LiquidCrystal *logger;

public:
    Estacionamiento(int capacidad, LiquidCrystal *logger)
    {
        this->counterCarros = CounterWithLimit(0, capacidad + 1, 0);
        this->logger = logger;
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

Estacionamiento estacionamiento(15, &lcd);

void setup()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    // Print a message to the LCD.
    lcd.print("hello, world!");
}

void loop()
{
    // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0):
    lcd.setCursor(0, 1);
    // print the number of seconds since reset:
    lcd.print(millis() / 1000);
}