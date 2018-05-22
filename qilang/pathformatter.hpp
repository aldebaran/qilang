#ifndef QILANG_PATHFORMATTER_HPP
#define QILANG_PATHFORMATTER_HPP

#include <string>
#include <qilang/api.hpp>

namespace qilang {

// TODO: document the purpose of this function, and the
// encoding used for input and output.
// (hint: current usage seems inconsistent).
QILANG_API std::string formatPath(const std::string& path);

}

#endif
