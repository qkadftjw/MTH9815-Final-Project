/**
 * funcs.hpp
 * This header file defines several utility functions and random number generators.
 * These include parsing date strings, generating unique identifiers,
 * handling bond information, parsing and formatting prices,
 * and working with time-related operations.
 *
 * @author Fangtong Wang
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>

#include "products.hpp"

using namespace std;
using namespace chrono;


/**
 * Gets the current system time as a string with millisecond precision.
 * @return A string representing the current time in the format "YYYY-MM-DD HH:MM:SS.sss".
 */
std::string CurrentTimeString() {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto secPart = time_point_cast<seconds>(now);
    auto msPart = duration_cast<milliseconds>(now - secPart);

    std::time_t rawTime = system_clock::to_time_t(now);
    char buffer[24];
    std::strftime(buffer, sizeof(buffer), "%F %T", std::localtime(&rawTime));

    std::ostringstream result;
    result << buffer << "." << std::setfill('0') << std::setw(3) << msPart.count();
    return result.str();
}

/**
 * Gets the current millisecond count within the current second.
 * @return The millisecond count as a long integer.
 */
long CurrentMillSecond() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto msPart = duration_cast<milliseconds>(now - time_point_cast<seconds>(now));
    return msPart.count();
}

/**
 * Generates a unique 12-character alphanumeric identifier.
 * @return A string representing the unique identifier.
 */
std::string GenerateUniqueId() {
    std::string baseChars;
    for (char c = '0'; c <= '9'; ++c) baseChars.push_back(c);
    for (char c = 'A'; c <= 'Z'; ++c) baseChars.push_back(c);

    unsigned long seed = static_cast<unsigned long>(std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(0.0, 1.0);

    std::string id;
    id.reserve(12);
    for (int i = 0; i < 12; ++i) {
        int index = static_cast<int>(dist(rng) * baseChars.size());
        id.push_back(baseChars[index % baseChars.size()]);
    }

    return id;
}

/**
 * Parses a date string in the format "YYYY/MM/DD" and converts it to a `year_month_day` object.
 * @param date_str The input date string.
 * @return A `year_month_day` object representing the parsed date.
 */
year_month_day parse_date_string(const std::string& date_str) {
    auto parse_component = [](const std::string& input, size_t& start, char delimiter) -> int {
        size_t end = input.find(delimiter, start);
        if (end == std::string::npos) {
            // Handle error appropriately here
        }
        int value = std::stoi(input.substr(start, end - start));
        start = end + 1;
        return value;
    };

    size_t position = 0;
    int year = parse_component(date_str, position, '/');
    int month = parse_component(date_str, position, '/');
    int day = std::stoi(date_str.substr(position));  // Parse the remaining part

    return std::chrono::year{year} / month / day;
}

/**
 * Retrieves the PV01 value for a specific US Treasury bond based on its CUSIP.
 * @param _cusip The CUSIP of the bond.
 * @return The PV01 value for the bond.
 * @throws std::invalid_argument if the CUSIP is unknown.
 */
double PV01Info(const std::string& _cusip) {
    static const std::unordered_map<std::string, double> pv01_map = {
        {"91282CLY5", 0.1854}, {"91282CMB4", 0.2738}, {"91282CMA6", 0.4389}, {"91282CLZ2", 0.5911},
        {"91282CLW9", 0.7910}, {"912810UF3", 1.2829}, {"912810UE6", 1.5956},
    };

    auto it = pv01_map.find(_cusip);
    if (it != pv01_map.end()) {
        return it->second;
    }

    throw std::invalid_argument("Unknown CUSIP");
}

/**
 * Retrieves bond information for a specific US Treasury bond based on its CUSIP.
 * @param _cusip The CUSIP of the bond.
 * @return A `Bond` object containing detailed information about the bond.
 * @throws std::invalid_argument if the CUSIP is unknown.
 */
Bond BondInfo(const std::string& _cusip) {
    static const std::unordered_map<std::string, Bond> bond_map = {
        {"91282CLY5", Bond("91282CLY5", CUSIP, "US2Y", 0.0425, parse_date_string("2026-11-30"))},
        {"91282CMB4", Bond("91282CMB4", CUSIP, "US3Y", 0.0400, parse_date_string("2027-12-15"))},
        {"91282CMA6", Bond("91282CMA6", CUSIP, "US5Y", 0.04125, parse_date_string("2029-11-30"))},
        {"91282CLZ2", Bond("91282CLZ2", CUSIP, "US7Y", 0.04125, parse_date_string("2031-11-30"))},
        {"91282CLW9", Bond("91282CLW9", CUSIP, "US10Y", 0.0425, parse_date_string("2034-11-15"))},
        {"912810UF3", Bond("912810UF3", CUSIP, "US20Y", 0.04625, parse_date_string("2044-11-15"))},
        {"912810UE6", Bond("912810UE6", CUSIP, "US30Y", 0.04500, parse_date_string("2054-11-15"))},
    };

    auto it = bond_map.find(_cusip);
    if (it != bond_map.end()) {
        return it->second;
    }

    throw std::invalid_argument("Unknown CUSIP");
}

/**
 * Parses a price string in the format "X-YZ+a" and converts it into a decimal value.
 * @param inputPrice The input price string.
 * @return The parsed price as a double.
 */
double ParsePrice(const std::string& inputPrice) {
    std::string partWhole, part32, part8;
    int dashCount = 0;

    for (char ch : inputPrice) {
        if (ch == '-') {
            dashCount++;
            continue;
        }
        if (dashCount == 0)
            partWhole.push_back(ch);
        else if (dashCount > 0 && dashCount < 3)
            part32.push_back(ch), dashCount++;
        else if (dashCount == 3)
            part8.push_back(ch == '+' ? '4' : ch);
    }

    double wholeVal = partWhole.empty() ? 0.0 : std::stod(partWhole);
    double val32 = part32.empty() ? 0.0 : std::stod(part32);
    double val8 = part8.empty() ? 0.0 : std::stod(part8);

    return wholeVal + val32 / 32.0 + val8 / 256.0;
}

/**
 * Formats a decimal price into a string representation in the format "X-YZ+a".
 * @param price The price as a double.
 * @return The formatted price string.
 */
std::string FormatPrice(double price) {
    int integerPart = static_cast<int>(std::floor(price));
    double fraction = price - integerPart;

    int fraction256 = static_cast<int>(std::floor(fraction * 256.0));
    int fraction32 = fraction256 / 8;
    int fraction8 = fraction256 % 8;

    std::ostringstream oss32;
    oss32 << (fraction32 < 10 ? "0" : "") << fraction32;
    std::string part8 = (fraction8 == 4) ? "+" : std::to_string(fraction8);

    std::ostringstream result;
    result << integerPart << "-" << oss32.str() << part8;
    return result.str();
}

#endif