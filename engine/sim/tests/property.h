// Property tests: RapidCheck inside doctest. (RapidCheck's own doctest bridge does not build
// against doctest 2.4.11, so this is the small bridge we use.)
#pragma once
#include <utility>

#include <doctest.h>
#include <rapidcheck.h>

template <class Property>
void property(const char* description, Property&& p) {
  CHECK_MESSAGE(rc::check(description, std::forward<Property>(p)), description);
}
