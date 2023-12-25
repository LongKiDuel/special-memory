#include <array>
#include <fmt/core.h>
#include <fmt/format.h>
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
std::string format_sql(pqxx::work &work, std::string tmplate, Ts &&...data) {
  std::array<std::string, sizeof...(data)> arr{
      work.quote(std::forward<Ts>(data))...};
  auto args = make_format_args_sql(arr);
  return fmt::vformat(tmplate, args);
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

  } catch (std::exception const &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
}