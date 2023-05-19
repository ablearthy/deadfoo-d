#include <iostream>
#include <memory>

#include <deadfood/storage/db_storage.hh>
#include <deadfood/storage/table_storage.hh>
#include <deadfood/scan/table_scan.hh>
#include <deadfood/core/schema.hh>

using namespace deadfood::core;
using namespace deadfood::storage;
using namespace deadfood::scan;

int main() {
  TableStorage storage;
  Schema schema;
  schema.AddField("a", field::kBoolField);
  schema.AddField("b", field::kIntField);
  schema.AddField("c", field::kFloatField);

  TableScan scan(storage, schema);

  scan.BeforeFirst();
  float x = 42.0;
  bool b = true;
  for (int i = 2; i < 10; ++i) {
    scan.Insert();
    scan.SetField("a", b);
    scan.SetField("b", i);
    scan.SetField("c", x);
    b = !b;
    x *= 42;
  }

  scan.BeforeFirst();
  while (scan.Next()) {
    std::cout << std::get<bool>(scan.GetField("a")) << ' '
              << std::get<int>(scan.GetField("b")) << ' '
              << std::get<float>(scan.GetField("c")) << '\n';
  }
  scan.BeforeFirst();
  while (scan.Next()) {
    if (!std::get<bool>(scan.GetField("a"))) {
      scan.Delete();
    }
  }
  std::cout << "--------\n";

  scan.BeforeFirst();
  while (scan.Next()) {
    std::cout << std::get<bool>(scan.GetField("a")) << ' '
              << std::get<int>(scan.GetField("b")) << ' '
              << std::get<float>(scan.GetField("c")) << '\n';
  }
}
