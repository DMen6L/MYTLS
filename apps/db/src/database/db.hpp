#ifndef DB
#define DB

#include <pqxx/pqxx>
#include <string>

std::string db_init_conn(
    const std::string &conn_file_path = "/home/dmensxl/projects/MYTLS/.env");

#endif
