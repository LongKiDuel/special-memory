#include "tiny-csv.h"
#include <iostream>
int main() {
  auto table = import_csv(std::cin);
  std::cout << export_csv(table);
}
