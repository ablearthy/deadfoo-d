add_library(deadfoo-d-libs
        lib.cc deadfood/storage/db_storage.cc deadfood/storage/db_storage.hh deadfood/storage/table_storage.cc deadfood/storage/table_storage.hh deadfood/core/schema.cc deadfood/core/schema.hh deadfood/core/field.cc deadfood/core/field.hh deadfood/scan/iscan.hh deadfood/storage/byte_buffer.cc deadfood/storage/byte_buffer.hh deadfood/core/row.cc deadfood/core/row.hh deadfood/core/row.cc deadfood/scan/table_scan.cc deadfood/scan/table_scan.hh)

target_include_directories(deadfoo-d-libs PUBLIC .)