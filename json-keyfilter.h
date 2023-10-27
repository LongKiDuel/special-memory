#pragma once
template <typename T, typename Key, typename... Keys>
T filter_json_key(const T &json, const Key &key, const Keys &...keys) {
  T result;
  if constexpr (sizeof...(keys) > 0) {
    result = filter_json_key(json, (keys, ...));
  }

  if (json.contains(key)) {
    result[key] = json[key];
  }

  return result;
}

template <typename T, typename Keys>
T filter_json_key_array(const T &json, const Keys &keys) {
  T result;

  for (auto &key : keys) {
    if (json.contains(key)) {
      result[key] = json[key];
    }
  }

  return result;
}

template <typename T, typename Keys>
T filter_json_drop_key_array(const T &json, const Keys &keys) {
  T result = json;

  for (auto &key : keys) {
    result.erase(key);
  }

  return result;
}
template <typename Json, typename Key, typename Transformer>
void transform_json_value(Json &json, const Key &key,
                          Transformer &&transformer) {
  //   if (json.contains(key)) {

  //   }
  json[key] = transformer(json[key]);
}