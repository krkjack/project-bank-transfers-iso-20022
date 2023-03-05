/*
 * MIT License
 * Copyright (c) 2017 Kevin Kirchner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * \file        libiban.cpp
 * \brief       Main source file for \p libiban
 * \author      Kevin Kirchner
 * \date        2017
 * \copyright   MIT LICENSE
 *
 * This source file is the main source file for \p libiban and implements the
 * classes and functions of the library.
 */

#include <iostream>
#include "libiban.h"
#include "utils.h"

namespace IBAN {

    /**
     * Constructor of \p IBANParseException.
     *
     * @param iban The IBAN string causing the exception
     */
    IBANParseException::IBANParseException(const std::string &iban) noexcept :
            m_iban(iban) {
        m_message = "Cannot parse IBAN " + m_iban;
    }

    /**
     * Destructor of \p IBANParseException
     */
    IBANParseException::~IBANParseException() {}

    /**
     * Overrides the function \p what() for this exception and returns a
     * string describing the error.
     *
     * @return A string describing the error
     */
    const char *IBANParseException::what() const noexcept {
        return m_message.c_str();
    }

    /**
     * Constructor for \p IBANInvalidCountryCodeException.
     *
     * @param countryCode The country code that caused the exception
     */
    IBANInvalidCountryCodeException::IBANInvalidCountryCodeException(
            const std::string &countryCode) noexcept :
            m_countryCode(countryCode){
        m_message = "Invalid country code " + m_countryCode;
    }

    /**
    * Destructor for \p IBANInvalidCountryCodeException.
    */
    IBANInvalidCountryCodeException::~IBANInvalidCountryCodeException() {}

    /**
     * Overrides the function \p what() for this exception and returns a
     * string describing the error.
     *
     * @return A string describing the error
     */
    const char* IBANInvalidCountryCodeException::what() const noexcept {
        return m_message.c_str();
    }

    // initialize country code map
    const map_t IBAN::m_countryCodes = {
        {"AL", 28}, {"AD", 24}, {"AT", 20}, {"AZ", 28}, {"BE", 16},
        {"BH", 22}, {"BA", 20}, {"BR", 29}, {"BG", 22}, {"CR", 22},
        {"HR", 21}, {"CY", 28}, {"CZ", 24}, {"DK", 18}, {"DO", 28},
        {"EE", 20}, {"FO", 18}, {"FI", 18}, {"FR", 27}, {"GE", 22},
        {"DE", 22}, {"GI", 23}, {"GR", 27}, {"GL", 18}, {"GT", 28},
        {"HU", 28}, {"IS", 26}, {"IE", 22}, {"IL", 23}, {"IT", 27},
        {"KZ", 20}, {"KW", 30}, {"LV", 21}, {"LB", 28}, {"LI", 21},
        {"LT", 20}, {"LU", 20}, {"MK", 19}, {"MT", 31}, {"MR", 27},
        {"MU", 30}, {"MC", 27}, {"MD", 24}, {"ME", 22}, {"NL", 18},
        {"NO", 15}, {"PK", 24}, {"PS", 29}, {"PL", 28}, {"PT", 25},
        {"RO", 24}, {"SM", 27}, {"SA", 24}, {"RS", 22}, {"SK", 24},
        {"SI", 19}, {"ES", 24}, {"SE", 24}, {"CH", 21}, {"TN", 24},
        {"TR", 26}, {"AE", 23}, {"GB", 22}, {"VG", 24}, {"BJ", 28},
        {"BF", 28}, {"BI", 16}, {"CM", 27}, {"CV", 25}, {"TL", 23},
        {"IR", 26}, {"CI", 28}, {"JO", 30}, {"SA", 24}, {"MG", 27},
        {"ML", 28}, {"MZ", 25}, {"QA", 29}, {"XK", 20}, {"SN", 28},
        {"LC", 32}, {"ST", 25}, {"UA", 29}, {"SC", 31}, {"IQ", 23},
        {"BY", 28}, {"SV", 28}, {"AO", 25}, {"CF", 27}, {"CG", 27},
        {"EG", 27}, {"DJ", 27}, {"DZ", 24}, {"GA", 27}, {"GQ", 27},
        {"GW", 25}, {"MA", 28}, {"NE", 28}, {"TD", 27}, {"TG", 28},
        {"KM", 27}, {"HN", 28}, {"NI", 32},
    };

    /**
     * Tries to create a new instance of \p IBAN from a string parameter. If
     * parsing fails, the methods throws an \p IBANParseException. This happens
     * if the string is too short to be an IBAN number or if it contains invalid
     * characters.
     *
     * Note that this does not guarantee the validity of the IBAN number. Call
     * \p validate() to test for validity.
     *
     * @param string The string to create an IBAN from
     * @return A new instance of \p IBAN
     */
    IBAN IBAN::createFromString(const std::string &string) {
        std::string s;
        std::string countryCode{}, accID{}, checkSum{};

        s = const_cast<std::string&>(string);
        trim(s);
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
        // too long or too short
        if (s.length() > 34 || s.length() < 5) {
            throw IBANParseException(string);
        }

        // first to chars are country code
        countryCode = s.substr(0, 2);
        if (!isalpha(countryCode[0]) || !isalpha(countryCode[1])) {
            throw IBANParseException(string);
        }
        // then two chars for the check sum
        checkSum = s.substr(2, 2);
        if (!std::isdigit(checkSum[0]) || !std::isdigit(checkSum[1])) {
            throw IBANParseException(string);
        }

        // rest is account ID
        accID = s.substr(4);
        for (auto ch : accID) {
            if (!isalnum(ch)) {
                throw IBANParseException(string);
            }
        }

        return IBAN(countryCode, accID, checkSum);
    }

    /**
     * Return the account identifier part of the IBAN number.
     *
     * @return The account identifier of the IBAN
     */
    std::string IBAN::getBBAN() const {
        return m_bban;
    }

    /**
     * Return the checksum part of the IBAN number.
     *
     * @return The checksum of the IBAN
     */
    std::string IBAN::getChecksum() const {
        return m_checkSum;
    }

    /**
     * Return the country code part of the IBAN number.
     *
     * @return The country code of the IBAN
     */
    std::string IBAN::getCountryCode() const {
        return m_countryCode;
    }

    /**
     * Returns the machine friendly formatting of the IBAN number which does
     * not contain any spaces.
     *
     * @return Machine friendly representation of the IBAN
     */
    std::string IBAN::getMachineForm() const {
        return m_countryCode + m_checkSum + m_bban;
    }

    /**
     * Returns the human readable formatting of the IBAN number which formats
     * the IBAN number into blocks of 4 characters.
     *
     * @return Human readable representation of the IBAN
     */
    std::string IBAN::getHumanReadable() const {
        std::stringstream result("");
        result << m_countryCode << m_checkSum;
        for (size_t i = 0; i < m_bban.length(); i += 4) {
            result << " " << m_bban.substr(i, 4);
        }
        return result.str();
    }

    /**
     * Validates the underlying object according to the IBAN format
     * specification and returns a boolean value indicating validation status.
     *
     * @return \p true if IBAN is valid, \p false otherwise
     */
    bool IBAN::validate() const {
        // invalid country code
        if (m_countryCodes.find(m_countryCode) == m_countryCodes.end()) {
            return false;
        }
        size_t expectedLength = m_countryCodes.find(m_countryCode)->second;

        // concat string and check length
        std::string checkString = m_bban + m_countryCode + m_checkSum;
        if (checkString.length() != expectedLength) {
            return false;
        }

        std::string numeric = makeNumerical(checkString);
        return getReminderForIBANString(numeric) == 1;
    }

    /**
     * This function generates a IBAN number with a given country code. The
     * resulting IBAN number will be a valid IBAN according to the specification.
     * This function will throw a \p IBANInvalidCountryCodeException if the
     * entered country codes is not valid.
     *
     * \b Note: The generated IBAN number is for testing purposes only. Do not
     * use them for banking, as only banks can generate and assign valid IBANs
     * for this purpose. Using a self-generated IBAN for banking will most
     * probably cause legal problems for you. I am in no way responsible for
     * the consequences resulting from using the generated numbers.
     *
     * @param countryCode The code of the country to generate a IBAN for
     * @return A newly generated valid IBAN
     */
    IBAN IBAN::generateIBAN(const std::string &countryCode) {
        if (m_countryCodes.find(countryCode) == m_countryCodes.end()) {
            throw IBANInvalidCountryCodeException(countryCode);
        }
        size_t ibanSize = m_countryCodes.find(countryCode)->second;

        // generate string and append country code and '00' (initial checksum)
        std::string ibanString = generateRandomString(ibanSize - 4);

        std::string numeric = makeNumerical(ibanString + countryCode + "00");
        auto checksum = std::to_string(98 - getReminderForIBANString(numeric));

        // pad checksum
        while (checksum.length() < 2) {
            checksum = "0" + checksum;
        }

        IBAN iban = createFromString(countryCode + checksum + ibanString);
        return iban;
    }
}
