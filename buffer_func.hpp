#include <utility>
#include <cstdint>

using read_buffer_func = std::add_pointer<void(std::uintptr_t address, std::uint8_t* buffer, std::size_t n)>::type;
using write_buffer_func = std::add_pointer<void(std::uintptr_t address, std::uint8_t* buffer, std::size_t n)>::type;
