#include <iostream>
#include <deadfood/database.hh>

#include <deadfood/lex/lex.hh>

#include <deadfood/parse/create_table_parser.hh>
#include <deadfood/parse/drop_table_parser.hh>
#include <deadfood/parse/insert_parser.hh>
#include <deadfood/parse/select_parser.hh>
#include <deadfood/parse/update_parser.hh>
#include <deadfood/parse/delete_parser.hh>

#include <deadfood/exec/create_table.hh>
#include <deadfood/exec/drop_table.hh>
#include <deadfood/exec/insert_into.hh>
#include <deadfood/exec/select.hh>
#include <deadfood/exec/update.hh>
#include <deadfood/exec/delete.hh>

#include <readline/readline.h>
#include <readline/history.h>

using namespace deadfood;

void ProcessQueryInternal(Database& db, const std::vector<lex::Token>& tokens) {
  if (IsKeyword(tokens[0], lex::Keyword::Create)) {  // create table query
    const auto q = parse::ParseCreateTableQuery(tokens);
    exec::ExecuteCreateTableQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Drop)) {  // drop table query
    const auto q = parse::ParseDropTableQuery(tokens);
    exec::ExecuteDropTableQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Update)) {  // update query
    const auto q = parse::ParseUpdateQuery(tokens);
    exec::ExecuteUpdateQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Delete)) {  // delete query
    const auto q = parse::ParseDeleteQuery(tokens);
    exec::ExecuteDeleteQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Insert)) {  // insert query
    const auto q = parse::ParseInsertQuery(tokens);
    exec::ExecuteInsertQuery(db, q);
  } else if (IsKeyword(tokens[0], lex::Keyword::Select)) {  // select query
    const auto q = parse::ParseSelectQuery(tokens);
    exec::ExecuteSelectQuery(db, q);
  } else {
    std::cout << "unknown query\n";
    return;
  }
}

int ProcessQuery(Database& db, const std::string& query) {
  if (query.starts_with(".exit")) {
    return -1;
  }
  std::vector<lex::Token> tokens;
  try {
    tokens = lex::Lex(query);
  } catch (const std::runtime_error& err) {
    std::cout << "[error (lex)] " << err.what() << '\n';
    return 1;
  }
  if (tokens.empty()) {
    std::cout << "expected some input\n";
    return 1;
  }

  try {
    ProcessQueryInternal(db, tokens);
  } catch (const parse::ParserError& e) {
    std::cout << "[error (parse)] " << e.what() << '\n';
    return 1;
  } catch (const std::runtime_error& err) {
    std::cout << "[error (runtime)] " << err.what() << '\n';
    return 1;
  }
  return 0;
}

int main(int argc, char** argv) {
  std::optional<char*> path;
  Database db;
  if (argc == 2) {
    path = argv[1];
    db = Load(path.value());
  } else {
    std::cout << "! in-memory\n";
  }

  char* query_buf;
  while ((query_buf = readline("> ")) != nullptr) {
    std::string query{query_buf};
    add_history(query_buf);

    free(query_buf);
    auto ret_code = ProcessQuery(db, query);
    if (ret_code == -1) {
      break;
    }
  }
  if (path.has_value()) {
    Dump(db, path.value());
  }
}