#include <psl/fstream.h>

#include <pine/fileio.h>
#include <pine/log.h>

namespace pine {

str read_str(str_view filename) {
  return psl::read_str(filename).or_([&]() { FATAL("Unable to read \"", filename, '"'); });
}

}  // namespace pine
