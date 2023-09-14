#include "User.h"
#include "drogon/orm/DbClient.h"
#include <string>
void api::v1::User::getInfo(
    const HttpRequestPtr &req,
    std::function<void(const HttpResponsePtr &)> &&callback, int userId) const {
  static auto db = drogon::orm::DbClient::newPgClient(
      "host=localhost port=5432 user=fix_db_user connect_timeout=10 "
      "password=fix_db_pass",
      10);
  // static auto db = drogon::orm::DbClient::newMysqlClient(
  //     "host=localhost port=3306 user=fix_db_user connect_timeout=10 "
  //     "password=fix_db_pass",
  //     10);
  db->execSqlSync("CREATE TABLE IF NOT EXISTS Users(name TEXT) ");
  db->execSqlSync("INSERT INTO Users(name) VALUES ($1) ",
                  std::to_string(userId));

  auto result = db->execSqlSync("SELECT * FROM Users");
  for(auto r: result){
    for(auto i: r){
      std::cout << i.name()<< ": " << i.c_str() <<"\n";
    }
  }
  
  Json::Value v;
  v["id"] = userId;
  callback(HttpResponse::newHttpJsonResponse(v));
}
