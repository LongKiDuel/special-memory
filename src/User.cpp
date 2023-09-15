#include "User.h"
#include "drogon/orm/DbClient.h"
#include <drogon/orm/Exception.h>
#include <drogon/orm/Result.h>
#include <string>
void api::v1::User::getInfo(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback, int userId) const {
  static auto db = drogon::orm::DbClient::newPgClient(
      "host=localhost port=5432 user=fix_db_user connect_timeout=10 "
      "password=fix_db_pass dbname=postgres",
      100);
  // static auto db = drogon::orm::DbClient::newMysqlClient(
  //     "host=localhost port=3306 user=fix_db_user connect_timeout=10 "
  //     "password=fix_db_pass",
  //     10);
  // static bool vser = [&] () { db->execSqlSync("DROP TABLE Users;"); return
  // true;}();
  // db->execSqlSync(
  //     "CREATE TABLE IF NOT EXISTS Users(name TEXT, register_time
  //     TIMESTAMP);");
  for (int i = 0; i < 100'000; i++) {

    // db->execSqlSync(
    // "INSERT INTO Users(name, register_time) VALUES ($1, NOW()); ",
    // std::to_string(userId + i));

    db->execSqlAsync(
        "INSERT INTO Users(name, register_time) VALUES ($1, NOW()); ",
        [](const orm::Result &) {}, [](const orm::DrogonDbException &e) {
          std::cout << e.base().what() <<"\n";
        },
        std::to_string(userId + i));
  }

  // auto result = db->execSqlSync("SELECT * FROM Users");
  // for(auto r: result){
  //   for(auto i: r){
  //     std::cout << i.name()<< ": " << i.c_str() <<"\n";
  //   }
  // }

  Json::Value v;
  v["id"] = userId;
  callback(HttpResponse::newHttpJsonResponse(v));
}
