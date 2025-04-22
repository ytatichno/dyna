#include "AnalysisResult.hpp"
#include <cstdint>
#include <memory>
#include <sstream>
#include <sys/types.h>
#include <vector>
namespace dyna {

class ExtraExchangesAnalysisResult : AnalysisResult {
  std::vector<unsigned> elements;
  std::shared_ptr<std::vector<uint64_t>> dims;
  // uint elementSize;
public:
  ExtraExchangesAnalysisResult(std::vector<unsigned> &&elements, std::shared_ptr<std::vector<uint64_t>> dims)
    : elements(elements), dims(dims)
  {}

  // Возвращает в строковом виде описание элементов массива
  std::string ToString() const {
    // todo fold by dims
    std::stringstream ss;
    ss << '[';
    for(const auto &e: elements){
      ss << e;
      ss << ", ";
    }
    ss << ']';
    return ss.str();
  }
};

}