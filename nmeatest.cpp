
#include "nmea.h"

#include <cmath>
#include <cstdio>
#include <cassert>

void IntegerParsing_TryParseCorrectInt32_Success()
{
    int32_t x = 0;
    bool isValid = false;

    isValid = parseInteger("-429", 4, x);
    assert(isValid);
    assert(-429 == x);

    isValid = parseInteger("73", 2, x);
    assert(isValid);
    assert(73 == x);
}

void IntegerParsing_TryParseInt32WithLetter_ErrorDetected()
{
    int32_t x = 0;
    bool isValid = false;

    isValid = parseInteger("23a34", 4, x);
    assert(!isValid);
}

void IntegerParsing_TryParseInt32WithDot_ErrorDetected()
{
    int32_t x = 0;
    bool isValid = false;

    isValid = parseInteger("23.34", 4, x);
    assert(!isValid);
}

void IntegerParsing_TryParseInt32Garbage_ErrorDetected()
{
    int32_t x = 0;
    bool isValid = false;

    isValid = parseInteger("hello world", 4, x);
    assert(!isValid);
}

void GpggaParsing_TryParseCorrectGpggaMessage_Success()
{
    NmeaGpggaMessage message;
    bool isValid = parseGpggaMessage("$GPGGA,102604.000,3150.7815,N,11711.9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);

    assert(isValid);
    assert(message.time.hours == 10);
    assert(message.time.minutes == 26);
    assert(message.time.seconds == 4);
    assert(fabs(message.latitude - (31.0 + (50.7815 / 60.0))) < 0.00001);
    assert(fabs(message.longitude - (117.0 + (11.9352 / 60.0))) < 0.00001);
    assert(NmeaGpggaFixStatus_GnssFix == message.fixStatus);
    assert(4 == message.numberOfSatellites);
    assert(fabs(message.altitude - 57.7) < 0.00001);
}

void GpggaParsing_TryParseCorrectGpggaMessageSouth_Success()
{
    NmeaGpggaMessage message;
    bool isValid = parseGpggaMessage("$GPGGA,102604.000,3150.7815,S,11711.9352,E,1,4,3.13,57.7,M,0.0,M,,*46\r\n", message);

    assert(isValid);
    assert(message.time.hours == 10);
    assert(message.time.minutes == 26);
    assert(message.time.seconds == 4);
    // South => Latitude is negative
    assert(fabs(message.latitude + (31.0 + (50.7815 / 60.0))) < 0.00001);
    assert(fabs(message.longitude - (117.0 + (11.9352 / 60.0))) < 0.00001);
    assert(NmeaGpggaFixStatus_GnssFix == message.fixStatus);
    assert(4 == message.numberOfSatellites);
    assert(fabs(message.altitude - 57.7) < 0.00001);
}

void GpggaParsing_TryParseCorrectGpggaMessageWest_Success()
{
    NmeaGpggaMessage message;
    bool isValid = parseGpggaMessage("$GPGGA,102604.000,3150.7815,N,11711.9352,W,1,4,3.13,57.7,M,0.0,M,,*49\r\n", message);

    assert(isValid);
    assert(message.time.hours == 10);
    assert(message.time.minutes == 26);
    assert(message.time.seconds == 4);
    assert(fabs(message.latitude - (31.0 + (50.7815 / 60.0))) < 0.00001);
    // West => Longitude is negative
    assert(fabs(message.longitude + (117.0 + (11.9352 / 60.0))) < 0.00001);
    assert(NmeaGpggaFixStatus_GnssFix == message.fixStatus);
    assert(4 == message.numberOfSatellites);
    assert(fabs(message.altitude - 57.7) < 0.00001);
}

void GpggaParsing_TryParseGpggaMessageWithInvalidTime_ErrorDetected()
{
    NmeaGpggaMessage message;
    bool isValid = true;

    isValid = parseGpggaMessage("$GPGGA,1a02604.000,3150.7815,N,11711.9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);
    assert(!isValid);
    isValid = parseGpggaMessage("$GPGGA,10260s4.000,3150.7815,N,11711.9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);
    assert(!isValid);
}

void GpggaParsing_TryParseGpggaMessageWithInvalidLatLng_ErrorDetected()
{
    NmeaGpggaMessage message;
    bool isValid = true;

    // Letter in latitude
    isValid = parseGpggaMessage("$GPGGA,102604.000,a3150.7815,N,11711.9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);
    assert(!isValid);

    // Letter in longitude
    isValid = parseGpggaMessage("$GPGGA,102604.000,3150.7815,N,11a711.9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);
    assert(!isValid);

    // Letter instead of dot in longitude
    isValid = parseGpggaMessage("$GPGGA,102604.000,3150.7815,N,11711a9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);
    assert(!isValid);

    // No dot in latitude
    isValid = parseGpggaMessage("$GPGGA,102604.000,31507815,N,11711.9352,E,1,4,3.13,57.7,M,0.0,M,,*5B\r\n", message);
    assert(!isValid);
}

void GxrmcParsing_TryParseCorrectGprmcMessage_Success()
{
    NmeaGxrmcMessage message;
    bool isValid = false;

    isValid = parseGxrmcMessage("$GPRMC,102739.000,A,3150.7825,N,11711.9369,E,0.00,303.62,111214,,,D*6A\r\n", message);

    assert(isValid);
    assert(message.positioningMode == NmeaGxrmcPositioningMode_DifferentialGnssFix);
    assert(message.validity == NmeaGxrmcValidity_Valid);
    assert(message.time.hours == 10);
    assert(message.time.minutes == 27);
    assert(message.time.seconds == 39);
    assert(message.date.day == 11);
    assert(message.date.month == 12);
    assert(message.date.year == 14);
    assert(fabs(message.latitude - (31.0 + (50.7825 / 60.0))) < 0.00001);
    assert(fabs(message.longitude - (117.0 + (11.9369 / 60.0))) < 0.00001);
    assert(fabs(message.speedOverGround - 0.0) < 0.00001);
    assert(fabs(message.courseOverGround - 303.62) < 0.00001);
}

void GxrmcParsing_TryParseCorrectGnrmcMessage_Success()
{
    NmeaGxrmcMessage message;
    bool isValid = false;

    isValid = parseGxrmcMessage("$GNRMC,102243.000,A,3150.7856,N,11711.9479,E,0.00,118.03,111214,,,D*71\r\n", message);

    assert(isValid);
    assert(message.positioningMode == NmeaGxrmcPositioningMode_DifferentialGnssFix);
    assert(message.validity == NmeaGxrmcValidity_Valid);
    assert(message.time.hours == 10);
    assert(message.time.minutes == 22);
    assert(message.time.seconds == 43);
    assert(message.date.day == 11);
    assert(message.date.month == 12);
    assert(message.date.year == 14);
    assert(fabs(message.latitude - (31.0 + (50.7856 / 60.0))) < 0.00001);
    assert(fabs(message.longitude - (117.0 + (11.9479 / 60.0))) < 0.00001);
    assert(fabs(message.speedOverGround - 0.0) < 0.00001);
    assert(fabs(message.courseOverGround - 118.03) < 0.00001);
}

int main()
{
    IntegerParsing_TryParseCorrectInt32_Success();
    IntegerParsing_TryParseInt32WithLetter_ErrorDetected();
    IntegerParsing_TryParseInt32WithDot_ErrorDetected();
    IntegerParsing_TryParseInt32Garbage_ErrorDetected();
    GpggaParsing_TryParseCorrectGpggaMessage_Success();
    GpggaParsing_TryParseGpggaMessageWithInvalidTime_ErrorDetected();
    GpggaParsing_TryParseGpggaMessageWithInvalidLatLng_ErrorDetected();
    GxrmcParsing_TryParseCorrectGprmcMessage_Success();
    GxrmcParsing_TryParseCorrectGnrmcMessage_Success();
    
    printf("\033[32mSUCCESS\033[00m\n\n");
}

