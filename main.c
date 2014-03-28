#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define A_MIN_POWER 50
#define A_MAX_POWER 200

#define B_MIN_POWER 60
#define B_MAX_POWER 200

#define STRENGTH_STEP 0.005

/*
 * Adjusts the strength in an attempt to ensure straight travel
 */
void adjustStrength(const int aCount, const int bCount, double* aStrength, double* bStrength);

/*
 * Returns a random double between the min and max
 */
double drand(double min, double max);

/*
 * Ensures that the value is kept between min and max (inclusive)
 */
void bound(double* value, double min, double max);


int main(int argc, const char* argv[])
{
    // initialize random seed
    srand(time(NULL));

    /*
     * The power is the pwm value
     */
    int aPower = A_MAX_POWER;
    int bPower = A_MAX_POWER;

    /*
     * How much desired pwm value should be used
     *
     * This is used to compensate for weaker/stronger motors
     */
    double aStrength = 1.0;
    double bStrength = 1.0;

    /*
     * How many counts are completed based on a given level of motor power and
     * time step
     *
     * The ratios simulate the interface between the motor and the encoder
     * Units: counts / motor power / second of applied motor power
     */
    double aRatio = drand(1.800, 2.000);
    double bRatio = drand(1.400, 1.600);

    /*
     * The current encoder counts
     *
     * The reason they are doubles is so we can have a simulation steps that
     * don't increment the counter by a full integer (more realistic).
     */
    double aCount = 0;
    double bCount = 0;

    /*
     * Amount of time between each simulation step
     */
    double step = 0.0008; // 800 microseconds

    /*
     * How far we want to go
     */
    int desiredCount = 100;

    /*
     * Average of how far we actually are
     */
    int actualCount = 0;

    /*
     * Distance we have left to cover (%)
     */
    double percentLeft = 1.0; // 100%

    while(actualCount < desiredCount) {
        /*
         * measure & calculate
         */
        aCount += (double)aPower * aStrength * aRatio * step;
        bCount += (double)bPower * bStrength * bRatio * step;

        actualCount = ((int)aCount + (int)bCount) / 2;
        percentLeft = 1.0 - ((double)actualCount / (double)desiredCount);
        aPower = (A_MAX_POWER - A_MIN_POWER) * percentLeft + A_MIN_POWER;
        bPower = (B_MAX_POWER - B_MIN_POWER) * percentLeft + B_MIN_POWER;

        printf("ratio(%.4f, %.4f) strength(%.4f, %.4f) power(% 5d, % 5d) count(% 4d, % 4d) = % 2d\n",
                aRatio, bRatio,
                aStrength, bStrength,
                aPower, bPower,
                (int)aCount, (int)bCount,
                ((int)aCount - (int)bCount));

        /*
         * adjust strength
         */
        adjustStrength((int)aCount, (int)bCount, &aStrength, &bStrength);

        /*
         * apply error
         */
        aRatio += drand(-0.0001, 0.0001); // +/- 1%
        bRatio += drand(-0.0001, 0.0001); // +/- 1%

        usleep(step * 1000000); // make the simulation run in ~real time
    }
}

void adjustStrength(const int aCount, const int bCount, double* aStrength, double* bStrength)
{
    if(aCount == bCount) {
        return;
    }

    double adjustment = STRENGTH_STEP * ((double)aCount - (double)bCount);

    *aStrength -= adjustment / 2.0;
    *bStrength += adjustment / 2.0;

    // make sure we stay in bounds
    bound(aStrength, 0, 1.0);
    bound(bStrength, 0, 1.0);
}

void bound(double* value, double min, double max)
{
    if(*value < min) {
        *value = min;        
    } else if(*value > max) {
        *value = max;
    }
}

double drand(double min, double max)
{
    double r = ((double)(rand() % 10000)) / 10000.0;

    return min + r * (max - min);
}
