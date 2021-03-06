#include <cstdint>
#include <fmt/core.h>
#include <fmt/chrono.h>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <vector>

template<typename Item1>
void
doit(const uint8_t* Data, std::size_t Size, int argsize)
{

  const auto N1 = sizeof(Item1);
  if (Size <= N1) {
    return;
  }
  Item1 item1{};
  if constexpr (std::is_same<Item1, bool>::value) {
    item1 = !!Data[0];
  } else {
    std::memcpy(&item1, Data, N1);
  }
  Data += N1;
  Size -= N1;

  // how many chars should be used for the argument name?
  if (argsize <= 0 || argsize >= Size) {
    return;
  }
  std::vector<char> argname(argsize + 1);
  std::memcpy(argname.data(), Data, argsize);
  Data += argsize;
  Size -= argsize;

  // allocates as tight as possible, making it easier to catch buffer overruns.
  // also, make it null terminated.
  std::vector<char> buf(Size + 1);
  std::memcpy(buf.data(), Data, Size);
  std::string message =
    fmt::format(buf.data(), fmt::arg(argname.data(), item1));
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
  // allocates as tight as possible, making it easier to catch buffer overruns
  std::vector<char> buf(Data, Data + Size);
  buf.resize(Size + 1, '\0');
  auto* b = std::localtime(&item);
  if (b) {
    std::string message = fmt::format(buf.data(), *b);
  }
}

// for dynamic dispatching to an explicit instantiation
template<typename Callback>
void
invoke(int index, Callback callback)
{
  switch (index) {
    case 0:
      callback(bool{});
      break;
    case 1:
      callback(char{});
      break;
    case 2:
      callback(short{});
      break;
    case 3:
      callback(int{});
      break;
    case 4:
      callback(long{});
      break;
    case 5:
      callback(float{});
      break;
    case 6:
      callback(double{});
      break;
    case 7:
      using LD = long double;
      callback(LD{});
      break;
  }
}

extern "C" int
LLVMFuzzerTestOneInput(const uint8_t* Data, std::size_t Size)
{

  if (Size <= 3) {
    return 0;
  }

  // switch types depending on the first byte of the input
  const auto first = Data[0] & 0x0F;
  const auto second = (Data[0] & 0xF0) >> 4;
  Data++;
  Size--;

  auto outer = [=](auto param1) { doit<decltype(param1)>(Data, Size, second); };

  try {
    invoke(first, outer);
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
