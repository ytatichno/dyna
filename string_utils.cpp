#include "string_utils.h"
#include <sstream>
using namespace std;

std::string string_substitute_vars(const std::string& str, const std::map<std::string, std::string>& vars)
{
  stringstream buf;
  size_t len = str.size();
  for (size_t i = 0; i < len;) {
    if (str[i] == '$') {
      //## variable substitution ##
      size_t b = i;
      size_t name_b = 0;
      size_t name_e = 0;
      if (str[++i] == '{') {
        // ## ${...} syntax ##
        name_b = i+1;
        while (++i < len) {
          if (str[i] == '}') {
            name_e = i++;
            break;
          }
        }
      }
      if (name_e > name_b) {
        // a correct variable name found
        string var_name = str.substr(name_b, name_e - name_b);
        auto it = vars.find(var_name);
        if (it != vars.end()) {
          buf << it->second;
        }
      }
      else {
        buf << str.substr(b, i - b);
      }
      continue;
    }
    if (str[i] == '\\') {
      ++i;
    }
    buf << str[i++];
  }
  return buf.str();
}
