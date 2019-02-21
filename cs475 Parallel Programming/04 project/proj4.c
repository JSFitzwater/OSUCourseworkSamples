#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define M_PI 3.14159265358979323846264338327950288


const float AVG_LOCUSTS = 0.75;  // My Average
const float AMP_LOCUSTS = 0.37;   // My Plus or Minus
const float RANDOM_LOCUSTS = 0.11;// My Plus or Minus Noise
const float GRAIN_GROWS_PER_MONTH = 8.0;
const float ONE_DEER_EATS_PER_MONTH = 0.5;
const float AVG_PRECIP_PER_MONTH = 6.0; // average
const float AMP_PRECIP_PER_MONTH = 6.0; // plus or minus
const float RANDOM_PRECIP = 2.0;        // plus or minus noise
const float AVG_TEMP = 50.0;    // average
const float AMP_TEMP = 20.0;    // plus or minus
const float RANDOM_TEMP = 10.0; // plus or minus noise
const float MIDTEMP = 40.0;
const float MIDPRECIP = 10.0;
//const unsigned int seed = 0;

    float nowHeight;  // grain height in inches
    float nowLocusts; // My Variable
    int   nowMonth;   // 0 - 11
    int   nowNumDeer; // number of deer in the current population
    float nowPrecip;  // inches of rain per month
    float nowTemp;    // temperature this month
    int   nowYear;    // 2017 - 2022


float Ranf( unsigned int seedp, float low, float high );
float SQR( float x );

void  GrainDeer(  );
void  Grain(  );
void  myAgent(  );
void  Watcher(  );


int main() {

    unsigned int seed = time(NULL);

    // Units of grain growth -- inches.
    // Units of temperature -- degrees Fahrenheit (°F).
    // Units of precipitation -- inches.

    //  -------------- 

    nowMonth = 1;       // starting date
    nowYear = 2017;     // starting time

    //  -------------- 

    //  temperature and precipitation are a function of the particular month
    float ang = ( 30.*(float)nowMonth + 15. ) * ( M_PI / 180. );

    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    nowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    nowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
    if( nowPrecip < 0. ){ nowPrecip = 0.; }

    //  -------------- 

    nowNumDeer = 2;     // starting state
    nowHeight = 10.;     // starting state

    nowLocusts = 0.25;  // starting state

    //  -------------- 

        printf( "\nTESTPRINT\nmonth+year: %d %d\ndeer: %d  locusts: %f  height: %f  precip: %f  temp: %f\nTESTPRINT\n\n\n", 
                nowMonth, nowYear, nowNumDeer, nowLocusts, nowHeight, nowPrecip, nowTemp );

    //  -------------- 

        omp_set_num_threads( 4 ); // same as # of sections
        #pragma omp parallel sections
        {
            #pragma omp section
            {
                GrainDeer(  );

            }
            #pragma omp section
            {
                Grain(  );
            }
            #pragma omp section
            {
                myAgent( ); // your own
                /* MYAGENT - In addition to this, you must add in some 
                 * other phenomenon that directly or indirectly controls
                 * the growth of the grain and/or the graindeer population. 
                 * Your choice of this is up to you. 
                 */
            }
            #pragma omp section
            {
                
                Watcher(  );
                
            }   // implied barrier -- all functions must return in order
            // to allow any of them to get past here
        }

    return 0;
}

    //  -------------- 

void GrainDeer( )
{
    while( nowYear < 2023 )
    {
        #pragma omp barrier
        // DoneComputing Barrier

        int newDeer;

        if( nowHeight < nowNumDeer )
        {
            newDeer = nowNumDeer - 1;

            if( newDeer < 0 ){ newDeer = 0; }
        }
        else {
            newDeer = nowNumDeer + 1;
        }

        #pragma omp barrier
        // DoneComputing Barrier

        nowNumDeer = newDeer;

        #pragma omp barrier
        // DoneAssigning Barrier

        #pragma omp barrier
        // DonePrinting Barrier
    }

    return;
}


void Grain(  )
{
    while( nowYear < 2023 )
    {
        #pragma omp barrier
        // DoneComputing Barrier

        float newHeight;
        float tempTemp;
        float tempPrecip;

        tempTemp = nowTemp;
        tempPrecip = nowPrecip;

        float tempFactorGrain = exp( -SQR( ( tempTemp - MIDTEMP ) / 10. ) );
        float precipFactorGrain = exp( -SQR( ( tempPrecip - MIDPRECIP ) / 10. ) );


        newHeight  = tempFactorGrain * precipFactorGrain * GRAIN_GROWS_PER_MONTH;

        newHeight -= (float)nowNumDeer * ONE_DEER_EATS_PER_MONTH;

        newHeight -= nowLocusts;

        if( newHeight < 0. ){ newHeight = 0.; }


        #pragma omp barrier
        // DoneComputing Barrier

        nowHeight = newHeight;

        #pragma omp barrier
        // DoneAssigning Barrier

        #pragma omp barrier
        // DonePrinting Barrier
    }

    return;
}


void myAgent(  )
{
    while( nowYear < 2023 )
    {
        #pragma omp barrier
        // DoneComputing Barrier

        float newLocusts;

        float ang = ( 30.*(float)nowMonth + 15. ) * ( M_PI / 180. );

        float tempLocusts = AVG_LOCUSTS - AMP_LOCUSTS * cos( ang );


        newLocusts = tempLocusts + Ranf( &seed, -RANDOM_LOCUSTS, RANDOM_LOCUSTS );

        if( newLocusts < 0. ){ newLocusts *= -1; }

        if( newLocusts > 2. ){ newLocusts -= 1; }


        #pragma omp barrier
        // DoneComputing Barrier

        nowLocusts = newLocusts;

        #pragma omp barrier
        // DoneAssigning Barrier

        #pragma omp barrier
        // DonePrinting Barrier
    }

    return;
}


void Watcher(  )
{
    while( nowYear < 2023 )
    {
        #pragma omp barrier
        // DoneComputing Barrier

        #pragma omp barrier
        // DoneComputing Barrier

        #pragma omp barrier
        // DoneAssigning Barrier

        printf( "month-year: %d-%d  height: %f  deer: %d  locusts: %f  precip: %f  temp: %f\n", 
                nowMonth, nowYear, nowHeight, nowNumDeer, nowLocusts, nowPrecip, nowTemp );

        nowMonth += 1;
        if( nowMonth == 13 )
        {
            nowMonth = 1;
            nowYear += 1;
        }


        //  temperature and precipitation are a function of the particular month
/**
        ADJUSTED "30." to "10." TO INCREASE GRAIN HEIGHT
        float ang = ( 30.*(float)nowMonth + 15. ) * ( M_PI / 180. );
**/
        float ang = ( 10.*(float)nowMonth + 15. ) * ( M_PI / 180. );

        float temp = AVG_TEMP - AMP_TEMP * cos( ang );
        nowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

        float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
        nowPrecip = precip + Ranf( &seed, -RANDOM_PRECIP, RANDOM_PRECIP );
        if( nowPrecip < 0. ){ nowPrecip = 0.; }


        #pragma omp barrier
        // DonePrinting Barrier
    }

    return;
}

    //  -------------- 

float Ranf( unsigned int seedp, float low, float high )
{
    time_t t;
    unsigned int tt;

	// float r = (float) rand_r( seedp ); // 0 - RAND_MAX
    // srand( seedp );
    tt = time(&t);
    srand(tt);
	float r = (float) rand( ); // 0 - RAND_MAX
	return( low + r * ( high - low ) / (float)RAND_MAX );
}

float SQR( float x ) { return x*x; }