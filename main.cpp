#include <bitset>
using Compact_bitset = std::bitset<64>;
int main() {
  Compact_bitset bitset;
  bitset[0];

  auto bytes = sizeof(bitset);
}
