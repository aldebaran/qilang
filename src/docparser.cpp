#include <qilang/docparser.hpp>
#include <exception>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/algorithm/string/join.hpp>
#include <qi/log.hpp>

qiLogCategory("qilang.docparser");

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    ParamDecl,
    (std::string, name)
    (std::string, description)
)

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    ReturnDecl,
    (std::string, description)
)

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    ThrowDecl,
    (std::string, name)
    (std::string, description)
)

namespace qilang {
  typedef boost::variant<ParamDecl, ReturnDecl, ThrowDecl> Decl;
  typedef std::vector<Decl> DeclVec;
}

BOOST_FUSION_DEFINE_STRUCT(
    (qilang),
    DocInternal,
    (std::vector<std::string>, description)
    (qilang::DeclVec, declarations)
)

namespace qilang {

class Filler : public boost::static_visitor<void>
{
public:
  Filler(Doc& doc)
    : _doc(doc)
  {}

  void operator()(const ParamDecl& p) const
  {
    std::pair<Doc::Parameters::iterator, bool> ret = _doc.parameters.insert(std::make_pair(p.name, p.description));
    if (!ret.second)
      qiLogWarning() << p.name << " documented multiple times";
  }
  void operator()(const ReturnDecl& p) const
  {
    if (!_doc.return_)
      _doc.return_ = p.description;
    else
      qiLogWarning() << "multiple \\return declaration";
  }
  void operator()(const ThrowDecl& p) const
  {
    std::pair<Doc::Parameters::iterator, bool> ret = _doc.throw_.insert(std::make_pair(p.name, p.description));
    if (!ret.second)
      qiLogWarning() << p.name << " documented multiple times";
  }

private:
  Doc& _doc;
};

template <typename Iterator>
Doc parseDoc(Iterator begin, Iterator end)
{
  namespace spirit = boost::spirit;
  using namespace boost::spirit::qi;
  using boost::phoenix::val;
  using boost::phoenix::construct;
  using boost::phoenix::bind;
  using boost::tuple;

  rule<Iterator, void()> declstart;
  declstart =
    lit("\\return") | lit("\\param") | lit("\\throw");

  rule<Iterator, void()> paragraph_end =
    lexeme[lit('\n') >> *blank >> (lit('\n') | &declstart)];

  rule<Iterator, std::string(), space_type> paragraph;
  paragraph %=
    lexeme[+(lit("\\\\")[ val('\\') ]
            | ((lit('\n')[ _val += " " ] | char_)
               - paragraph_end))];

  rule<Iterator, std::vector<std::string>(), space_type> description;
  rule<Iterator, ReturnDecl(), space_type> return_decl;
  rule<Iterator, ParamDecl(), space_type> param_decl;
  rule<Iterator, ThrowDecl(), space_type> throw_decl;
  rule<Iterator, Decl(), space_type> decl;
  rule<Iterator, DocInternal(), space_type> start;

  BOOST_SPIRIT_DEBUG_NODE(description);
  BOOST_SPIRIT_DEBUG_NODE(paragraph_end);
  BOOST_SPIRIT_DEBUG_NODE(declstart);
  BOOST_SPIRIT_DEBUG_NODE(paragraph);
  BOOST_SPIRIT_DEBUG_NODE(return_decl);
  BOOST_SPIRIT_DEBUG_NODE(param_decl);
  BOOST_SPIRIT_DEBUG_NODE(throw_decl);
  BOOST_SPIRIT_DEBUG_NODE(decl);
  BOOST_SPIRIT_DEBUG_NODE(start);

  description %= *(!declstart >> paragraph);

  return_decl %= lit("\\return") > paragraph;

  throw_decl %= lit("\\throw") > lexeme[ +(char_ - ' ') ] > paragraph;

  param_decl %= lit("\\param") > lexeme[ +(char_ - ' ') ] > paragraph;

  decl %=
    return_decl
    | param_decl
    | throw_decl;

  start %= description > *decl;

  on_error<fail>(start, std::cout
      << val("Error: expecting `")
      << _4
      << val("', got `")
      << construct<std::string>(_3, _2)
      << val("'")
      << std::endl
      );

  DocInternal outint;
  bool b = phrase_parse(begin, end, start, space, outint);
  if (!b)
    throw std::runtime_error("parse error");

  if (begin != end)
  {
    std::ostringstream ss;
    ss << "garbage: " << std::string(begin, end);
    throw std::runtime_error(ss.str());
  }

  Doc out;

  if (!outint.description.empty())
  {
    std::size_t dot = outint.description[0].find('.');
    out.brief = std::string(outint.description[0], 0, dot);

    out.description = boost::algorithm::join(outint.description, "\n\n");
  }

  BOOST_FOREACH(const Decl& decl, outint.declarations) {
    boost::apply_visitor(Filler(out), decl);
  }

  return out;
}

Doc parseDoc(const std::string& str) {
  return parseDoc(str.begin(), str.end());
}

}
