#include "db.hpp"
#include "env.hpp"
#include <format>
#include <pqxx/pqxx>
#include <string>
#include <unordered_map>

pqxx::connection db_init_conn(const std::string &conn_file_path) {
  std::unordered_map<std::string, std::string> env = read_env(conn_file_path);
  // TODO: save conn_str as URI
  auto conn_str = std::format("host={} port={} dbname={} user={} password={}",
                              env["DB_HOST"], env["DB_PORT"], env["DB_NAME"],
                              env["DB_USER"], env["DB_PASSWORD"]);

  pqxx::connection conn(conn_str);
  return conn;
}
