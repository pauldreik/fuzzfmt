#include <cstdint>
#include <fmt/core.h>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <fmt/time.h>

template<typename Item>
void
doit(const uint8_t* Data, std::size_t Size)
{
  const auto N = sizeof(Item);
  if (Size <= N) {
    return;
  }
  Item item{};
  if constexpr (std::is_same<Item, bool>::value) {
    item = !!Data[0];
  } else {
    std::memcpy(&item, Data, N);
  }
  Data += N;
  Size -= N;
  // allocates as tight as possible, making it easier to catch buffer overruns
  std::vector<char> buf(Data, Data + Size);
  buf.resize(Size + 1, '\0');
  std::string message = fmt::format(buf.data(), item);
}

void
doit_time(const uint8_t* Data, std::size_t Size)
{
  using Item = std::time_t;
  const auto N = sizeof(Item);
  if (Size <= N) {
    return;
  }
  Item item{};
  std::memcpy(&item, Data, N);
  Data += N;
  Size -= N;
  // allocates as tight as possible, making it easier to catch buffer overruns.
  // also, make it null terminated.
  std::vector<char> buf(Size+1);
  std::memcpy(buf.data(),Data,Size);
  auto* b = std::localtime(&item);
  if (b) {
    std::string message = fmt::format(buf.data(), *b);
  }
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size)
{

  if (Size <= 3) {
    return 0;
  }

  const auto first = Data[0];
  Data++;
  Size--;

  try {
    switch (first) {
      case 0:
        doit<bool>(Data, Size);
        break;
      case 1:
        doit<char>(Data, Size);
        break;
      case 2:
        doit<short>(Data, Size);
        break;
      case 3:
        doit<int>(Data, Size);
        break;
      case 4:
        doit<long>(Data, Size);
        break;
      case 5:
        doit<float>(Data, Size);
        break;
      case 6:
        doit<double>(Data, Size);
        break;
      case 7:
        doit<long double>(Data, Size);
        break;
      case 8:
        doit_time(Data, Size);
        break;
      default:
        break;
    }
  } catch (std::exception& e) {
  }
  return 0;
}

#ifdef IMPLEMENT_MAIN
#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>
int
main(int argc, char* argv[])
{
  for (int i = 1; i < argc; ++i) {
    std::ifstream in(argv[i]);
    assert(in);
    in.seekg(0, std::ios_base::end);
    const auto pos = in.tellg();
    in.seekg(0, std::ios_base::beg);
    std::vector<char> buf(pos);
    in.read(buf.data(), buf.size());
    assert(in.gcount() == pos);
    LLVMFuzzerTestOneInput((const uint8_t*)buf.data(), buf.size());
  }
}
#endif
