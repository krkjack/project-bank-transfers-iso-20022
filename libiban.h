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
 * \file        libiban.h
 * \brief       Main header file for \p libiban
 * \author      Kevin Kirchner
 * \date        2017
 * \copyright   MIT LICENSE
 *
 * This header file is the main header file for \p libiban and declares the
 * classes and functions of the library.
 */

#ifndef LIBIBAN_LIBIBAN_H
#define LIBIBAN_LIBIBAN_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <memory>

namespace IBAN {

/// Used as a shortcut for the country codes map type
typedef std::unordered_map<std::string, size_t> map_t;

/// Exception to be thrown when parsing an IBAN string fails
class IBANParseException : public std::exception {

private:
    /// Holds the IBAN number causing the error
    std::string m_iban;
    /// The exception message
    std::string m_message;

public:
    IBANParseException(const std::string& iban) noexcept;
    virtual ~IBANParseException();
    virtual const char* what() const noexcept;

    /// Copy constructor for \p IBANParseException. Uses the default copy
    /// constructor
    IBANParseException(const IBANParseException& other)=default;
};

/// Exception to be thrown when a country code is not valid
class IBANInvalidCountryCodeException : std::exception {

private:
    /// Holds the country code causing the error
    std::string m_countryCode;
    /// The exception message
    std::string m_message;

public:
    IBANInvalidCountryCodeException(const std::string& countryCode) noexcept;
    virtual ~IBANInvalidCountryCodeException();
    virtual const char* what() const noexcept;

    /// Copy constructor for \p IBANInvalidCountryCodeException. Uses the
    /// default copy constructor
    IBANInvalidCountryCodeException(const IBANInvalidCountryCodeException& other)=default;
};

/// Main class of the library
class IBAN {

private:
    /// Holds the IBAN's country code
    std::string m_countryCode {};
    /// Holds the IBAN's Basic Bank Account Number
    std::string m_bban {};
    /// Holds the IBAN's check sum
    std::string m_checkSum {};
    IBAN(const std::string& cCode, const std::string& bban,
         const std::string& checkSum) : m_countryCode(cCode),
                                  m_bban(bban),
                                  m_checkSum(checkSum) {}

public:
    /// Copy constructor for \p IBAN. Uses the default copy constructor
    IBAN(const IBAN&)=default;
    ~IBAN() {}
    IBAN& operator=(IBAN other);
    bool operator==(const IBAN& other) const;
    bool operator!=(const IBAN& other) const;
    friend std::ostream& operator<<(std::ostream& stream, const IBAN& elem);
    static IBAN createFromString(const std::string& string);
    static IBAN generateIBAN(const std::string& countryCode);
    std::string getCountryCode() const;
    std::string getBBAN() const;
    std::string getChecksum() const;
    std::string getHumanReadable() const;
    std::string getMachineForm() const;
    bool validate() const;

    /// Static map mapping country codes to required IBAN length; must be
    /// initialized in a source file.
    static const map_t m_countryCodes;

    /**
     * Swaps \p first and \p second by swapping their member variables' values.
     *
     * @param first First instance of \p IBAN
     * @param second Second instance of \p IBAN
     */
    friend void swap(IBAN& first, IBAN& second) {
        using std::swap;
        swap(first.m_checkSum, second.m_checkSum);
        swap(first.m_bban, second.m_bban);
        swap(first.m_countryCode, second.m_countryCode);
    }

}; // end of class IBAN

/**
 * Overloads the comparison operator ==.
 *
 * @param other The object to compare with
 * @return \p true if both objects are equal, \p false otherwise
 */
inline bool IBAN::operator==(const IBAN &other) const {
    return ((this->getCountryCode() == other.getCountryCode()) &&
            (this->getChecksum() == other.getChecksum()) &&
            (this->getBBAN() == other.getBBAN()));
}

/**
 * Overloads the comparison operator !=.
 *
 * @param other The object to compare with
 * @return \p true if the objects are not equal, \p false if they are equal
 */
inline bool IBAN::operator!=(const IBAN &other) const {
    return !(*this == other);
}

/**
 * Implements assignment operator.
 *
 * @param other The other instance to assign
 * @return An instance of \p IBAN
 */
inline IBAN& IBAN::operator=(IBAN other) {
    IBAN temp(other);
    swap(*this, temp);
    return *this;
}

/**
 * Overrides the stream operator << for IBAN.
 *
 * @param stream The stream to write to
 * @param elem The element to write to the stream
 * @return The stream written to
 */
inline std::ostream& operator<<(std::ostream& stream, const IBAN& elem) {
    stream << "IBAN (" << elem.getHumanReadable() << ")";
    return stream;
}

} // end of namespace IBAN

#endif //LIBIBAN_LIBIBAN_H
