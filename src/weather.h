/****************************************************************************
 *             ___________.__               .__                             *
 *             \_   _____/|  | ___.__. _____|__|__ __  _____                *
 *              |    __)_ |  |<   |  |/  ___/  |  |  \/     \               *
 *              |        \|  |_\___  |\___ \|  |  |  /  Y Y  \              *
 *             /_______  /|____/ ____/____  >__|____/|__|_|  /              *
 *                     \/      \/         \/     Engine    \/               *
 *                       A SMAUG Derived Game Engine.                       *
 * ------------------------------------------------------------------------ *
 * Elysium Engine Copyright 1999-2009 by Steven Loar                        *
 * Elysium Engine Development Team: Kayle (Steven Loar), Venia, Scoyn,      *
 *                                  and Mikon.                              *
 * ------------------------------------------------------------------------ *
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame                    *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider                    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus, Scryn, Rennard,    *
 * Swordbearer, Gorog, Grishnakh, Nivek, Tricops and Fireblade              *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *	                     Weather System Header                             *
 ****************************************************************************
 *     Base Weather Model Copyright (c) 2007 Chris Jacobson                 *
 ****************************************************************************/

/*
 * This might not have all the bells and whistles I'd intended, and it might be 
 * missing some of the things people wanted. But I did the best I could, and it's
 * sure as hell better than the crap that used to be there. -Kayle
 */

//Change these values to expand or contract your weather map according to your world size.
const int WEATHER_SIZE_X      = 3; //number of cells wide
const int WEATHER_SIZE_Y      = 3; //number of cells tall

//Hemisphere defines.
const int HEMISPHERE_NORTH    = 0;
const int HEMISPHERE_SOUTH    = 1;
const int HEMISPHERE_MAX      = 2;

//Climate defines - Add more if you want, but make sure you add appropriate data to the
// system itself in EnforceClimateConditions()
const int CLIMATE_RAINFOREST  = 0;
const int CLIMATE_SAVANNA     = 1;
const int CLIMATE_DESERT      = 2;
const int CLIMATE_STEPPE      = 3;
const int CLIMATE_CHAPPARAL   = 4;
const int CLIMATE_GRASSLANDS  = 5;
const int CLIMATE_DECIDUOUS   = 6;
const int CLIMATE_TAIGA       = 7;
const int CLIMATE_TUNDRA      = 8;
const int CLIMATE_ALPINE      = 9;
const int CLIMATE_ARCTIC      = 10;
const int MAX_CLIMATE         = 11;

//So it can be utilized from other parts of the code
extern struct WeatherCell	weatherMap[WEATHER_SIZE_X][WEATHER_SIZE_Y];
extern struct WeatherCell	weatherDelta[WEATHER_SIZE_X][WEATHER_SIZE_Y];

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
