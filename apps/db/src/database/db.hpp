#ifndef DB
#define DB

#include <pqxx/pqxx>
#include <string>

pqxx::connection db_init_conn(const std::string &conn_file_path = ".env");

#endif
