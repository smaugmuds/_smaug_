/*
                     R E A L M S    O F    D E S P A I R  !
   ___________________________________________________________________________
  //            /                                                            \\
 [|_____________\   ********   *        *   ********   *        *   *******   |]
 [|   \\._.//   /  **********  **      **  **********  **      **  *********  |]
 [|   (0...0)   \  **********  ***    ***  **********  ***    ***  *********  |]
 [|    ).:.(    /  ***         ****  ****  ***    ***  ***    ***  ***        |]
 [|    {o o}    \  *********   **********  **********  ***    ***  *** ****   |]
 [|   / ' ' \   /   *********  *** ** ***  **********  ***    ***  ***  ****  |]
 [|-'- /   \ -`-\         ***  ***    ***  ***    ***  ***    ***  ***   ***  |]
 [|   .VxvxV.   /   *********  ***    ***  ***    ***  **********  *********  |]
 [|_____________\  **********  **      **  **      **  **********  *********  |]
 [|             /  *********   *        *  *        *   ********    *******   |]
  \\____________\____________________________________________________________//
     |                                                                     |
     |    --{ [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame }--    |
     |_____________________________________________________________________|
     |                                                                     |
     |                    -*- Weather System Header -*-                    |
     |_____________________________________________________________________|
     |                                                                     |
     | Elysium Engine Copyright 1999-2009 by Steven Loar                   |
     | Elysium Engine Development Team: Kayle (Steven Loar), Venia, Scoyn, |
     |                                   and Mikon.                        |
     | Base Weather Model Copyright (c) 2007 Chris Jacobson                |
     |_____________________________________________________________________|
    //                                                                     \\
   [|  SMAUG 2.0 © 2014-2015 Antonio Cao (@burzumishi)                      |]
   [|                                                                       |]
   [|  AFKMud Copyright 1997-2007 by Roger Libiez (Samson),                 |]
   [|  Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),       |]
   [|  Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,            |]
   [|  Xorith, and Adjani.                                                  |]
   [|  All Rights Reserved. External contributions from Remcon, Quixadhal,  |]
   [|  Zarius and many others.                                              |]
   [|                                                                       |]
   [|  SMAUG 1.4 © 1994-1998 Thoric/Altrag/Blodkai/Narn/Haus/Scryn/Rennard  |]
   [|  Swordbearer/Gorog/Grishnakh/Nivek/Tricops/Fireblade/Edmond/Conran    |]
   [|                                                                       |]
   [|  Merc 2.1 Diku Mud improvments © 1992-1993 Michael Chastain, Michael  |]
   [|  Quan, and Mitchell Tse. Original Diku Mud © 1990-1991 by Sebastian   |]
   [|  Hammer, Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, Katja    |]
   [|  Nyboe. Win32 port Nick Gammon.                                       |]
    \\_____________________________________________________________________//
*/

/*
 * This might not have all the bells and whistles I'd intended, and it might be 
 * missing some of the things people wanted. But I did the best I could, and it's
 * sure as hell better than the crap that used to be there. -Kayle
 */

//Change these values to expand or contract your weather map according to your world size.
#define WEATHER_SIZE_X 3 //number of cells wide
#define WEATHER_SIZE_Y 3 //number of cells tall

//Hemisphere defines.
#define HEMISPHERE_NORTH    0
#define HEMISPHERE_SOUTH    1
#define HEMISPHERE_MAX      2

//Climate defines - Add more if you want, but make sure you add appropriate data to the
// system itself in EnforceClimateConditions()
#define CLIMATE_RAINFOREST  0
#define CLIMATE_SAVANNA     1
#define CLIMATE_DESERT      2
#define CLIMATE_STEPPE      3
#define CLIMATE_CHAPPARAL   4
#define CLIMATE_GRASSLANDS  5
#define CLIMATE_DECIDUOUS   6
#define CLIMATE_TAIGA       7
#define CLIMATE_TUNDRA      8
#define CLIMATE_ALPINE      9
#define CLIMATE_ARCTIC      10
#define MAX_CLIMATE         11

//So it can be utilized from other parts of the code
/*
struct WeatherCell weatherMap[WEATHER_SIZE_X][WEATHER_SIZE_Y];
struct WeatherCell weatherDelta[WEATHER_SIZE_X][WEATHER_SIZE_Y];
*/

struct WeatherCell
{
   int climate;        // Climate flag for the cell
   int hemisphere;     // Hemisphere flag for the cell
   int temperature;    // Fahrenheit because I'm American, by god
   int pressure;       // 0..100 for now, later change to barometric pressures
   int cloudcover;     // 0..100, amount of clouds in the sky
   int humidity;       // 0+
   int precipitation;  // 0..100
   int energy;			// 0..100 Storm Energy, chance of storm.
   /*
   *  Instead of a wind direction we use an X/Y speed
   *  It makes the math below much simpler this way.
   *  Its not hard to determine a basic cardinal direction from this
   *  If you want to, a good rule of thumb is that if one directional
   *  speed is more than double that of the other, ignore it; that is
   *  if you have speed X = 15 and speed Y = 3, the wind is obviously
   *  to the east.  If X = 15 and Y = 10, then its a south-east wind. 
   */
   int windSpeedX;    //  < 0 = west, > 0 = east
   int windSpeedY;    //  < 0 = north, > 0 = south
};

/*
*	This is the Weather Map.  It is a grid of cells representing X-MAX_INPUT_LENGTHe square
*	areas of weather
*/
struct WeatherCell	weatherMap[WEATHER_SIZE_X][WEATHER_SIZE_Y];

/*
*	This is the Weather Delta Map.  It is used to accumulate changes to be
*	applied to the Weather Map.  Why accumulate changes then apply them, rather
*	than just change the Weather Map as we go?
*	Because doing that can mean a change just made to a neighbor can
*	immediately cause ANOTHER change to a neighbor, causing things
*	to get out of control or causing cascading weather, propagating much
*	faster and unpredictably (in a BAD unpredictable way)
*	Instead, we determine all the changes that should occur based on the current
*	'snapshot' of weather, than apply them all at once!
*/
struct WeatherCell	weatherDelta[WEATHER_SIZE_X][WEATHER_SIZE_Y];

//Defines
void InitializeWeatherMap( void );
void UpdateWeather( void );
void RandomizeCells( void );
void save_weathermap( void );
bool load_weathermap( void );

int get_hemisphere( char *type );
int get_climate( char *type );

struct WeatherCell *getWeatherCell( AREA_DATA *pArea );

void IncreaseTemp( struct WeatherCell *cell, int change );
void DecreaseTemp( struct WeatherCell *cell, int change );
void IncreasePrecip( struct WeatherCell *cell, int change );
void DecreasePrecip( struct WeatherCell *cell, int change );
void IncreasePressure( struct WeatherCell *cell, int change );
void DecreasePressure( struct WeatherCell *cell, int change );
void IncreaseEnergy( struct WeatherCell *cell, int change );
void DecreaseEnergy( struct WeatherCell *cell, int change );
void IncreaseCloudCover( struct WeatherCell *cell, int change );
void DecreaseCloudCover( struct WeatherCell *cell, int change );
void IncreaseHumidity( struct WeatherCell *cell, int change );
void DecreaseHumidity( struct WeatherCell *cell, int change );
void IncreaseWindX( struct WeatherCell *cell, int change );
void DecreaseWindX( struct WeatherCell *cell, int change );
void IncreaseWindY( struct WeatherCell *cell, int change );
void DecreaseWindY( struct WeatherCell *cell, int change );

int getCloudCover( struct WeatherCell *cell );
int getTemp( struct WeatherCell *cell );
int getEnergy( struct WeatherCell *cell );
int getPressure( struct WeatherCell *cell );
int getHumidity( struct WeatherCell *cell );
int getPrecip( struct WeatherCell *cell );
int getWindX( struct WeatherCell *cell );
int getWindY( struct WeatherCell *cell );

bool isExtremelyCloudy( int cloudCover );
bool isModeratelyCloudy( int cloudCover );
bool isPartlyCloudy( int cloudCover );
bool isCloudy( int cloudCover );
bool isSwelteringHeat( int temp );
bool isVeryHot( int temp );
bool isHot( int temp );
bool isWarm( int temp );
bool isTemperate( int temp );
bool isCool( int temp );
bool isChilly( int temp );
bool isCold( int temp );
bool isFrosty( int temp );
bool isFreezing( int temp );
bool isReallyCold( int temp );
bool isVeryCold( int temp );
bool isExtremelyCold( int temp );
bool isStormy( int energy );
bool isHighPressure( int pressure );
bool isLowPressure( int pressure );
bool isExtremelyHumid( int humidity );
bool isModeratelyHumid( int humidity );
bool isMinorlyHumid( int humidity );
bool isHumid( int humidity );
bool isTorrentialDownpour( int precip );
bool isRainingCatsAndDogs( int precip );
bool isPouring( int precip );
bool isRaingingHeavily( int precip );
bool isDownpour( int precip );
bool isRainingSteadily( int precip );
bool isRaining( int precip );
bool isRainingLightly( int precip );
bool isDrizzling( int precip );
bool isMisting( int precip );
bool isCalmWindE( int windx );
bool isBreezyWindE( int windx );
bool isBlusteryWindE( int windx );
bool isWindyWindE( int windx );
bool isGustyWindE( int windx );
bool isGaleForceWindE( int windx );
bool isCalmWindW( int windx );
bool isBreezyWindW( int windx );
bool isBlusteryWindW( int windx );
bool isWindyWindW( int windx );
bool isGustyWindW( int windx );
bool isGaleForceWindW( int windx );
bool isCalmWindN( int windy );
bool isBreezyWindN( int windy );
bool isBlusteryWindN( int windy );
bool isWindyWindN( int windy );
bool isGustyWindN( int windy );
bool isGaleForceWindN( int windy );
bool isCalmWindS( int windy );
bool isBreezyWindS( int windy );
bool isBlusteryWindS( int windy );
bool isWindyWindS( int windy );
bool isGustyWindS( int windy );
bool isGaleForceWindS( int windy );
