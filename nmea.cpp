
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

#include "nmea.h"

#include <cstring>
#include <ctime>
#include <type_traits>

static_assert(sizeof(float) == 4, "This code requires a float size of 4 bytes.");
static_assert(sizeof(double) == 8, "This code requires a double size of 8 bytes.");

template <typename T>
bool parseInteger(const char *chars, uint32_t length, T &result)
{
    result = 0;

    if (0 == length) {
        return true;
    }

    bool negative = false;

    for (uint32_t i = 0; i < length; i++) {
        if (std::is_signed<T>::value) {
            // Check sign
            if (i == 0 && chars[i] == '-') {
                negative = true;
                continue;
            }
        }

        // Get digit
        int32_t digit = (int32_t)(chars[i] - '0');

        // If it isn't a digit, return false
        if (digit > 9 || digit < 0) {
            return false;
        }

        result *= 10;
        result += digit;
    }

    if (std::is_signed<T>::value) {
        if (negative) {
            result = -result;
        }
    }

    return true;
}

bool parseInteger(const char *chars, uint32_t length, int32_t &result) {
    return parseInteger<int32_t>(chars, length, result);
}

bool parseDouble(const char *chars, uint32_t length, double &result)
{
    result = 0.0;

    int32_t integerPart = 0;
    double fractional = 0.1;
    bool dotFound = false;
    bool negative = false;

    for (uint32_t i = 0; i < length; i++) {
        // Check sign
        if (i == 0 && chars[i] == '-') {
            negative = true;
            continue;
        }

        // Check if this is a dot
        if (chars[i] == '.') {
            // If dot is already found, return false
            if (dotFound) {
                return false;
            }

            dotFound = true;
            result += (double) integerPart;
            continue;
        }

        // Get digit
        int32_t digit = (int32_t)(chars[i] - '0');

        // If it isn't a digit, return false
        if (digit > 9 || digit < 0) {
            return false;
        }

        if (!dotFound) {
            integerPart *= 10;
            integerPart += digit;
        } else {
            if (0 != digit) {
                result += (fractional * (double) (digit));
            }
            fractional /= 10.0;
        }
    }

    if (negative) {
        result = -result;
    }

    return true;
}

bool parseNmeaLatLng(const char *chars, uint32_t length, double &resultInDegrees)
{
    resultInDegrees = 0.0;

    if (0 == length) {
        return true;
    }

    // Find dot
    uint32_t dotIndex;
    for (dotIndex = 0; dotIndex < length && chars[dotIndex] != '.'; dotIndex++) {
    }

    if (dotIndex < 2 || chars[dotIndex] != '.') {
        return false;
    }

    int32_t integerDegrees = 0;
    double minutes = 0.0;

    // Parse degrees as 32-bit integer
    if (!parseInteger<int32_t>(chars, dotIndex - 2, integerDegrees)) {
        return false;
    }

    // Parse minutes as double
    if (!parseDouble(chars + dotIndex - 2, length - dotIndex + 2, minutes)) {
        return false;
    }

    // Calculate result in degrees
    resultInDegrees = (static_cast<double>(integerDegrees) + (minutes / 60.0));
    return true;
}

extern "C" {

static inline uint8_t charToHex(char c)
{
    uint8_t x = static_cast<uint8_t>(c - '0');
    if (x > 9) {
        x = static_cast<uint8_t>(c - 'A' + 10);
    }
    return x;
}

bool parseGpggaMessage(const char *chars, NmeaGpggaMessage &msg)
{
    uint8_t i;
    uint8_t colons = 0;
    uint8_t previousColon = 0;
    uint8_t calculatedChecksum = 0;
    uint8_t receivedChecksum = 0;
    uint8_t asteriskPosition = 0;

    memset(&msg, 0, sizeof(NmeaGpggaMessage));

    for (i = 0; chars[i] != 0 && chars[i] != '\r'; i++) {
        if (chars[i] == '*') {
            asteriskPosition = i;
        }
        if (i != 0 && asteriskPosition == 0) {
            calculatedChecksum ^= *reinterpret_cast<const uint8_t *>(chars + i);
        }
        if (chars[i] != ',') {
            continue;
        }

        char previousChar = chars[i - 1];
        double val;

        switch (colons) {
        case 1:
            // Time stamp

            if (!parseInteger(chars + previousColon + 1, 2, msg.time.hours)) {
                return false;
            }
            if (!parseInteger(chars + previousColon + 3, 2, msg.time.minutes)) {
                return false;
            }
            if (!parseInteger(chars + previousColon + 5, 2, msg.time.seconds)) {
                return false;
            }

            break;
        case 2:
            // Latitude in format ‘ddmm.mmmm’ (degree and minutes)

            if (!parseNmeaLatLng(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            msg.latitude = val;

            break;
        case 3:
            // North / South

            if (previousChar == NmeaDirection_North) {
            } else if (previousChar == NmeaDirection_South) {
                msg.latitude = (-msg.latitude);
            } else if (previousChar == ',') {
                break;
            } else {
                return false;
            }

            break;
        case 4:
            // Longitude in format ‘dddmm.mmmm’ (degree and minutes)

            if (!parseNmeaLatLng(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            msg.longitude = val;

            break;
        case 5:
            // East / West

            if (previousChar == NmeaDirection_East) {
            } else if (previousChar == NmeaDirection_West) {
                msg.longitude = (-msg.longitude);
            } else if (previousChar == ',') {
                break;
            } else {
                return false;
            }

            break;
        case 6:
            // Fix quality

            switch (previousChar) {
            case NmeaGpggaFixStatus_Invalid:
            case NmeaGpggaFixStatus_GnssFix:
            case NmeaGpggaFixStatus_DgpsFix:
            case NmeaGpggaFixStatus_EstimatedMode:
                msg.fixStatus = static_cast<NmeaGpggaFixStatus>(previousChar);
                break;
            default:
                return false;
            }

            break;
        case 7:
            // Number of satellites

            if (!parseInteger<decltype(msg.numberOfSatellites)>(chars + previousColon + 1, i - previousColon - 1, msg.numberOfSatellites)) {
                return false;
            }

            break;
        case 9:
            // Altitude in meters according to WGS84 ellipsoid

            if (!parseDouble(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            msg.altitude = val;

            break;
        }

        previousColon = i;
        colons++;
    }

    // Get received checksum
    receivedChecksum = charToHex(chars[asteriskPosition + 1]);
    receivedChecksum *= 16;
    receivedChecksum += charToHex(chars[asteriskPosition + 2]);

    // Compare checksums
    bool result = receivedChecksum == calculatedChecksum;
    return result;
}

bool parseGxrmcMessage(const char *chars, NmeaGxrmcMessage &msg)
{
    uint8_t i;
    uint8_t colons = 0;
    uint8_t previousColon = 0;
    uint8_t calculatedChecksum = 0;
    uint8_t receivedChecksum = 0;
    uint8_t asteriskPosition = 0;

    memset(&msg, 0, sizeof(NmeaGpggaMessage));

    for (i = 0; chars[i] != 0 && chars[i] != '\r'; i++) {
        if (chars[i] == '*') {
            asteriskPosition = i;
        }
        if (i != 0 && asteriskPosition == 0) {
            calculatedChecksum ^= *reinterpret_cast<const uint8_t *>(chars + i);
        }
        if (chars[i] != ',') {
            continue;
        }

        char previousChar = chars[i - 1];
        char nextChar = chars[i + 1];
        double val;

        switch (colons) {
        case 1:
            // Time stamp

            if (!parseInteger(chars + previousColon + 1, 2, msg.time.hours)) {
                return false;
            }
            if (!parseInteger(chars + previousColon + 3, 2, msg.time.minutes)) {
                return false;
            }
            if (!parseInteger(chars + previousColon + 5, 2, msg.time.seconds)) {
                return false;
            }

            break;
        case 2:
            // Data valid

            switch (previousChar) {
            case NmeaGxrmcValidity_Invalid:
            case NmeaGxrmcValidity_Valid:
                msg.validity = static_cast<NmeaGxrmcValidity>(previousChar);
                break;
            case ',':
                break;
            default:
                return false;
            }

            break;
        case 3:
            // Latitude in format ‘ddmm.mmmm’ (degree and minutes)

            if (!parseNmeaLatLng(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            msg.latitude = val;

            break;
        case 4:
            // North / South

            if (previousChar == NmeaDirection_North) {
            } else if (previousChar == NmeaDirection_South) {
                msg.latitude = (-msg.latitude);
            } else if (previousChar == ',') {
                break;
            } else {
                return false;
            }

            break;
        case 5:
            // Longitude in format ‘dddmm.mmmm’ (degree and minutes)

            if (!parseNmeaLatLng(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            msg.longitude = val;

            break;
        case 6:
            // East / West

            if (previousChar == NmeaDirection_East) {
            } else if (previousChar == NmeaDirection_West) {
                msg.longitude = (-msg.longitude);
            } else if (previousChar == ',') {
                break;
            } else {
                return false;
            }

            break;
        case 7:
            // Speed over ground in knots (we convert it to km/h)

            if (!parseDouble(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            // 1 knot = 1.852 km/h
            msg.speedOverGround = val / 1.852;

            break;
        case 8:
            // Course over ground in degree

            if (!parseDouble(chars + previousColon + 1, i - previousColon - 1, val)) {
                return false;
            }
            msg.courseOverGround = val;

            break;
        case 9:
            // Date in format 'ddmmyy'

            if (!parseInteger(chars + previousColon + 1, 2, msg.date.day)) {
                return false;
            }
            if (!parseInteger(chars + previousColon + 3, 2, msg.date.month)) {
                return false;
            }
            if (!parseInteger(chars + previousColon + 5, 2, msg.date.year)) {
                return false;
            }

            break;
        case 10:
            // Magnetic variation in degree (not being output by L76)
            break;
        case 11:
            // After 11th colon: Magnetic variation E/W indicator (not being output by L76)

            // After 12th colon: Positioning mode
            // NOTE: the switch does not go into case 12, so need to handle it here

            switch (nextChar) {
            case NmeaGxrmcPositioningMode_NoFix:
            case NmeaGxrmcPositioningMode_AutonomousGnssFix:
            case NmeaGxrmcPositioningMode_DifferentialGnssFix:
                msg.positioningMode = static_cast<NmeaGxrmcPositioningMode>(nextChar);
                break;
            case ',':
                break;
            default:
                return false;
            }

            break;
        }

        previousColon = i;
        colons++;
    }

    // Get received checksum
    receivedChecksum = charToHex(chars[asteriskPosition + 1]);
    receivedChecksum *= 16;
    receivedChecksum += charToHex(chars[asteriskPosition + 2]);

    // Compare checksums
    bool result = receivedChecksum == calculatedChecksum;
    return result;
}
}

