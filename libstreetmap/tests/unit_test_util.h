#pragma once

#include <UnitTest++/UnitTest++.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "m3.h"
#include "LatLon.h" 

// Previously 200 but it caused some crashes as the failure message can't be more than 1024
// char failureStr[1024]; in  DeferredTestResult.h
#ifndef MAX_VEC_PRINT
#define MAX_VEC_PRINT 40
#endif

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
    os << "{";
    for(size_t i = 0; i < vec.size(); i++) {
        os << vec[i];
#ifdef MAX_VEC_PRINT
        if(i > MAX_VEC_PRINT) {
            os << "...";
            break;
        }
#endif
        if(i != vec.size() - 1) {
            os << ", " ;
        }
    }
    os << "}";
    return os;
}


#ifdef TURN_TYPE
// overloading ostream << to print enum class TurnType
inline std::ostream& operator<<(std::ostream& os, const TurnType& turn) {
    if (turn == TurnType::STRAIGHT)
        os << "TurnType::STRAIGHT";
    else if (turn == TurnType::RIGHT)
        os << "TurnType::RIGHT";
    else if (turn == TurnType::LEFT)
        os << "TurnType::LEFT";
    else 
        os << "TurnType::NONE";

    return os;
}
#endif

namespace ece297test {

template<typename T>
float relative_error(T A, T B) {
    if (A == B) {
        return 0.;
    }

    if (fabs(B) > fabs(A)) {
        return fabs((A - B) / B);
    } else {
        return fabs((A - B) / A);
    }
}

template<typename T>
float absolute_error(T A, T B) {
    return fabs(A - B);
}

template< typename Expected, typename Actual >
void CheckEqual(UnitTest::TestResults& results, Expected const& expected, Actual const& actual, UnitTest::TestDetails const& details, const char* stringified_actual)
{
    if (!(expected == actual))
    {
        UnitTest::MemoryOutStream stream;
        stream << "Expected " << expected << " but was " << actual << " for expression: " << stringified_actual;

        results.OnTestFailure(details, stream.GetText());
    }
}

template< typename Expected, typename Actual, typename Tolerance >
void CheckRelativeError(UnitTest::TestResults& results, Expected const& expected, Actual const& actual, Tolerance const& tolerance,
                UnitTest::TestDetails const& details, const char* stringified_actual)
{
    const auto rel_error = relative_error(expected, actual);
    if (!UnitTest::AreClose(0.0, rel_error, tolerance))
    {
        UnitTest::MemoryOutStream stream;
        stream << "Expected (" << expected << ") and actual (" << actual << ") should be within "
            << tolerance*100.0 << "%, but were " << rel_error*100 << "% different. For expression: " << stringified_actual;

        results.OnTestFailure(details, stream.GetText());
    }
}

template<typename Range>
auto sorted(Range&& r) -> decltype(std::forward<Range>(r)) {
    std::sort(r.begin(), r.end());
    return std::forward<Range>(r);
}

}

#ifdef ECE297_TIME_CONSTRAINT
    #error unit_test_util.h redefines ECE297_TIME_CONSTRAINT
#endif

#ifdef ECE297_CHECK
    #error unit_test_util.h redefines ECE297_CHECK
#endif

#ifdef ECE297_CHECK_EQUAL
    #error unit_test_util.h redefines ECE297_CHECK_EQUAL
#endif

#ifdef ECE297_CHECK_RELATIVE_ERROR
    #error unit_test_util.h redefines ECE297_CHECK_RELATIVE_ERROR
#endif

//We define a new time constraint ECE297_TIME_CONSTRAINT which
//is identical to UNITTEST_TIME_CONSTRAINT.
#define ECE297_TIME_CONSTRAINT(ms) \
    UnitTest::TimeConstraint unitTest__timeConstraint__(ms, m_details, __LINE__)

// A version of CHECK that prints stringified value parameter with a message
#define ECE297_CHECK(value) \
    do \
    { \
        try { \
            if (!UnitTest::Check(value)) \
                UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), "This should be true: " #value); \
        } \
        catch (...) { \
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), \
                    "Unhandled exception in CHECK(" #value ")"); \
        } \
    } while (0)

// A version of CHECK_EQUAL that prints stringified actual parameter with a message
#define ECE297_CHECK_EQUAL(expected, actual) \
    do \
    { \
        try { \
            ece297test::CheckEqual(*UnitTest::CurrentTest::Results(), expected, actual, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), #actual); \
        } \
        catch (...) { \
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), \
                    "Unhandled exception in ECE297_CHECK_EQUAL(" #expected ", " #actual ")"); \
        } \
} while (0)

#define ECE297_CHECK_RELATIVE_ERROR(expected, actual, tolerance) \
    do \
    { \
        try { \
            ece297test::CheckRelativeError(*UnitTest::CurrentTest::Results(), expected, actual, tolerance, UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), #actual); \
        } \
        catch (...) { \
            UnitTest::CurrentTest::Results()->OnTestFailure(UnitTest::TestDetails(*UnitTest::CurrentTest::Details(), __LINE__), \
                    "Unhandled exception in ECE297_CHECK_RELATIVE_ERROR(" #expected ", " #actual ", " #tolerance ")"); \
        } \
    } while (0)
