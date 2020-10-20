#include "catch.hpp"

#include <exception>
#include <string>
#include <string_view>

namespace piranha{
    namespace test{

template <typename T>
class ExceptionMatcher : public Catch::MatcherBase<T>
 {
    
    public:
    ExceptionMatcher(std::string const & i):m_expected(i) {}
    
    bool match(T const& e) const override
    {
        return std::string_view(e.what()).find(m_expected) != std::string::npos;
    }
    
    std::string describe() const override {
        std::ostringstream ss;
        ss << "Exception has value of " << m_expected;
        return ss.str();
    }
    private:
    const std::string m_expected;
};
    }
}
