#include <qilang/pathformatter.hpp>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <qi/path.hpp>


namespace qilang {


std::string formatPath(const std::string& path)
{
  std::string res = path;
  // the case doesn't matter on windows
#ifdef _WIN32
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
#endif
  // TODO: when boost will be upgraded, replace normalize() by lexically_normal()
  return boost::filesystem::path(res, qi::unicodeFacet()).make_preferred().normalize().string(qi::unicodeFacet());
}


}
