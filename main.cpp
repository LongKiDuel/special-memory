#include <array>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include <string>

template <typename T, std::size_t... I>
auto make_format_args_helper(const std::array<T, sizeof...(I)> &arr,
                             std::index_sequence<I...>) {
  return fmt::make_format_args(arr[I]...);
}

template <typename T, std::size_t N>
auto make_format_args_sql(const std::array<T, N> &arr) {
  return make_format_args_helper(arr, std::make_index_sequence<N>{});
}
template <typename... Ts>
// Only works for format values in sqlite.
std::string format_sql(pqxx::work &work, std::string tmplate, Ts &&...data) {
  std::array<std::string, sizeof...(data)> arr{
      work.quote(std::forward<Ts>(data))...};
  auto args = make_format_args_sql(arr);
  return fmt::vformat(tmplate, args);
}

std::string get_hostname() {
  std::ifstream file{"/proc/sys/kernel/hostname"};
  std::string name;
  file >> name;
  return name;
}
int main() {
  try {
    // Connect to the database.  In practice we may have to pass some
    // arguments to say where the database server is, and so on.
    // The constructor parses options exactly like libpq's
    // PQconnectdb/PQconnect, see:
    // https://www.postgresql.org/docs/10/static/libpq-connect.html
    pqxx::connection c{getenv("db")};
    // Start a transaction.  In libpqxx, you always work in one.
    pqxx::work w(c);
    // work::exec1() executes a query returning a single row of data.
    // We'll just ask the database to return the number 1 to us.
    pqxx::row r = w.exec1("SELECT 1");
    // Commit your transaction.  If an exception occurred before this
    // point, execution will have left the block, and the transaction will
    // have been destroyed along the way.  In that case, the failed
    // transaction would implicitly abort instead of getting to this point.
    w.commit();
    // Look at the first and only field in the row, parse it as an integer,
    // and print it.
    //
    // "r[0]" returns the first field, which has an "as<...>()" member
    // function template to convert its contents from their string format
    // to a type of your choice.
    std::cout << r[0].as<int>() << std::endl;

    {
      pqxx::work w(c);

      pqxx::row r = w.exec1(format_sql(w, "SELECT {}, {};", 42, 32));
      for (auto &&c : r) {
        std::cout << c.c_str() << " ";
      }
      std::cout << "\n";
    }
    {
      pqxx::work w(c);

      auto rows =
          w.exec(format_sql(w, "SELECT tablename from pg_catalog.pg_tables;"));
      int count{};
      for (auto r : rows) {
        if (count == 0) {
          for (auto &&c : r) {
            std::cout << c.name() << " ";
          }
          std::cout << "\n";
        }

        for (auto &&c : r) {
          std::cout << c.c_str() << " ";
        }
        std::cout << "\n";
        count++;
      }
    }

    {

      pqxx::work w(c);
      std::string hostname = get_hostname();
      int machine_id{};
      {
        auto count = w.exec1(format_sql(
            w, "SELECT count(*) FROM machines WHERE name = {};", hostname));
        if (count[0].as<int>() == 0) {
          w.exec0(format_sql(w, "INSERT INTO machines (name) VALUES ({});",
                             hostname));
        }

        auto id = w.exec1(format_sql(
            w, "SELECT id FROM machines WHERE name = {};", hostname));
        machine_id = id[0].as<int>();
      }
      // CURRENT_TIMESTAMP == now()
      w.exec0("SET TIMEZONE='America/Los_angeles';");
      std::cout << w.exec1("SELECT now() - interval '2 hours 30 minutes' AS "
                           "two_hour_30_min_go;")[0]
                       .get<std::string>()
                       .value()
                << "\n";
      w.exec(format_sql(
          w,
          "INSERT INTO file_changes (machine_id, filename,filepath,filesize, "
          "last_modify_time, last_check_time) "
          "VALUES ({}, {}, {}, {}, now(), LOCALTIMESTAMP);",
          machine_id, "abx.txt", "/root/abx.txt", 523));
      auto rows = w.exec(fmt::format("SELECT * from {};", "file_changes"));
      int count{};
      for (auto r : rows) {
        if (count == 0) {
          for (auto &&c : r) {
            std::cout << c.name() << " ";
          }
          std::cout << "\n";
        }

        for (auto &&c : r) {
          std::cout << c.c_str() << " ";
        }
        std::cout << "\n";
        count++;
      }
      w.commit();
    }

  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}