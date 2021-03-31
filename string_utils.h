#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_
#include <map>
#include <string>

// Replace variables with their values.
std::string string_substitute_vars(const std::string& str, const std::map<std::string, std::string>& vars);

#endif //_STRING_UTILS_H_
