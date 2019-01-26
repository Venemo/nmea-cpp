
// This file is part of the C++ NMEA library.
// Copyright (c) 2016-2019 Timur Kristóf
// Licensed to you under the terms of the MIT license.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifdef __cplusplus
#    include <cstdint>
#else
#    include "stdint.h"
#endif // __cplusplus

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define NMEA_PACKED __attribute__((packed))

enum NMEA_PACKED NmeaGpggaFixStatus
{
    NmeaGpggaFixStatus_Invalid = '0',
    NmeaGpggaFixStatus_GnssFix = '1',
    NmeaGpggaFixStatus_DgpsFix = '2',
    NmeaGpggaFixStatus_EstimatedMode = '6',
};

static_assert(sizeof(NmeaGpggaFixStatus) == 1, "Size of NmeaGpggaFixStatus is expected to be 1.");

enum NMEA_PACKED NmeaDirection
{
    NmeaDirection_North = 'N',
    NmeaDirection_South = 'S',
    NmeaDirection_East = 'E',
    NmeaDirection_West = 'W',
};

static_assert(sizeof(NmeaDirection) == 1, "Size of NmeaDirection is expected to be 1.");

enum NMEA_PACKED NmeaGxrmcValidity
{
    NmeaGxrmcValidity_Invalid = 'V',
    NmeaGxrmcValidity_Valid = 'A',
};

static_assert(sizeof(NmeaGxrmcValidity) == 1, "Size of NmeaGxrmcValidity is expected to be 1.");

enum NMEA_PACKED NmeaGxrmcPositioningMode
{
    NmeaGxrmcPositioningMode_NoFix = 'N',
    NmeaGxrmcPositioningMode_AutonomousGnssFix = 'A',
    NmeaGxrmcPositioningMode_DifferentialGnssFix = 'D',
};

static_assert(sizeof(NmeaGxrmcPositioningMode) == 1, "Size of NmeaGxrmcPositioningMode is expected to be 1.");

typedef struct NMEA_PACKED NmeaTime
{
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} NmeaTime;

static_assert(sizeof(NmeaTime) == 3, "Size of NmeaTime is expected to be 3.");

typedef struct NMEA_PACKED NmeaDate
{
    uint8_t day;
    uint8_t month;
    uint8_t year;
} NmeaDate;

static_assert(sizeof(NmeaDate) == 3, "Size of NmeaDate is expected to be 3.");

typedef struct NMEA_PACKED NmeaGpggaMessage
{
    double latitude;
    double longitude;
    double altitude;
    NmeaTime time;
    uint8_t numberOfSatellites;
    NmeaGpggaFixStatus fixStatus;
} NmeaGpggaMessage;

static_assert(sizeof(NmeaGpggaMessage) == 29, "Size of NmeaGpggaMessage is expected to be 29.");

typedef struct NMEA_PACKED NmeaGxrmcMessage
{
    double latitude;
    double longitude;
    double courseOverGround;
    double speedOverGround;
    NmeaTime time;
    NmeaDate date;
    NmeaGxrmcValidity validity;
    NmeaGxrmcPositioningMode positioningMode;
} NmeaGxrmcMessage;

static_assert(sizeof(NmeaGxrmcMessage) == 40, "Size of NmeaGxrmcMessage is expected to be 40.");

extern bool parseInteger(const char *chars, uint32_t length, int32_t &result);

extern bool parseGpggaMessage(const char *chars, NmeaGpggaMessage &msg);

extern bool parseGxrmcMessage(const char *chars, NmeaGxrmcMessage &msg);

#ifdef __cplusplus
}
#endif // __cplusplus

