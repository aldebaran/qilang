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
  return boost::filesystem::path(res).make_preferred().string(qi::unicodeFacet());
}


}
