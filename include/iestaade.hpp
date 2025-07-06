#pragma once
#include <boost/json/src.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "rododendrs.hpp"

namespace iestaade {

const char KEY_PATH_DELIMITER = '/';

boost::json::value value_from_json(const std::string& file_path,
                                   const std::string& key_path)
{
    const std::ifstream t(file_path);
    if (!t.good()) {
        throw std::runtime_error("file not found: \"" + file_path + "\"");
    }

    std::stringstream buffer;
    buffer << t.rdbuf();
    boost::json::value v = boost::json::parse(buffer.str());
    assert(!v.is_null());

    std::vector<std::string> tokens;
    std::string token;
    std::stringstream ss(key_path);
    while (getline(ss, token, KEY_PATH_DELIMITER)) {
        tokens.push_back(token);
    }
    assert(!tokens.empty());

    for (size_t i = 0; i < tokens.size(); i++) {
        if (!v.as_object().if_contains(tokens[i])) {
            throw std::runtime_error("field not found: \"" + key_path +
                                     "\" in \"" + file_path + "\"");
        }
        v = v.at(tokens[i]);
    }
    return v;
}

template <typename T>
const T from_json(const std::string& file_path,
                  const std::string& key_path,
                  bool optional          = false,
                  const T& default_value = T())
{
    try {
        const boost::json::value json_v = value_from_json(file_path, key_path);
        if constexpr (std::is_same_v<T, bool>) {
            return json_v.get_bool();
        }
        else if constexpr (std::is_arithmetic_v<T>) {
            return json_v.to_number<T>();
        }
        else if constexpr (std::is_same_v<T, std::string>) {
            return json_v.as_string().c_str();
        }
        else if constexpr (rododendrs::is_range_v<T>) {
            // T = rododendrs::Range<U>
            using U = rododendrs::range_value_type_t<T>;
            T v;
            if (json_v.as_object().if_contains("min")) {
                v.min = json_v.at("min").to_number<U>();
            }
            if (json_v.as_object().if_contains("max")) {
                v.max = json_v.at("max").to_number<U>();
            }
            if (json_v.as_object().if_contains("step_min")) {
                v.step_min = json_v.at("step_min").to_number<U>();
            }
            if (json_v.as_object().if_contains("step_max")) {
                v.step_max = json_v.at("step_max").to_number<U>();
            }
            return v;
        }
        else {
            throw std::runtime_error("unsupported type for from_json");
        }
    }
    catch (const std::runtime_error&) {
        if (optional) {
            return default_value;
        }
        throw;
    }
}

/**
 * @brief reads data from a csv file and populates a vector of structs.
 *
 * reads data from a csv file specified by `file_path` and populates
 * the vector `v` with instances of `TStruct`.
 * each row in the csv file corresponds to an instance of `TStruct`,
 * and each column corresponds to a member of `TStruct`.
 *
 * @tparam TStruct  the type of the struct to populate.
 * @tparam TMembers the types of the members of the struct in the column order.
 * @param file_path the path to the csv file.
 * @param v         the vector to populate with instances of `TStruct`.
 * @param separator the character used to separate columns in the CSV file
 * (default is ',').
 * @throws std::runtime_error tf the file cannot be opened or if the number of
 * columns in the CSV file does not match the number of members in `TStruct`.
 */
template <typename TStruct, typename... TMembers>
void struct_from_csv(const std::string& file_path,
                     std::vector<TStruct>& v,
                     const char separator = ',')
{
    std::ifstream f(file_path);
    if (!f.good()) {
        throw std::runtime_error("file not found: \"" + file_path + "\"");
    }

    // skip header
    std::string line;
    std::getline(f, line);

    // read the data
    while (std::getline(f, line)) {
        std::stringstream line_stream(line);
        TStruct row;
        std::string cell;

        // lambda function to read and assign each cell
        // to the corresponding member
        // - [&] capture all local variables by reference
        auto read_and_assign = [&](auto& member) {
            std::getline(line_stream, cell, separator);
            std::stringstream(cell) >> member;
        };

        // use a fold expression ( ...) to call the lambda for each member
        // ref: https://en.cppreference.com/w/cpp/language/fold
        // - offset - offset of the member from the beginning of the TStruct
        // - reinterpret_cast - c++ cast type for unsafe low-level type
        // conversions
        // - remove_reference_t - remove reference from the type (rvalue,
        // lvalue)
        size_t offset = 0;
        ((read_and_assign(
                  *reinterpret_cast<std::remove_reference_t<TMembers>*>(
                          reinterpret_cast<char*>(&row) + offset)),
          offset += sizeof(TMembers)),
         ...);

        v.push_back(row);
    }

    f.close();
}

/**
 * @brief reads data from a csv file and populates a 2d vector of type T.
 */
template <typename T>
void vector_from_csv(const std::string& file_path,
                     std::vector<std::vector<T>>& v,
                     const char separator = ',')
{
    std::ifstream f(file_path);
    if (!f.good()) {
        throw std::runtime_error("file not found: \"" + file_path + "\"");
    }

    // skip header
    std::string line;
    std::getline(f, line);

    // read the data
    while (std::getline(f, line)) {
        std::stringstream line_stream(line);
        std::vector<T> row;
        std::string cell;

        while (std::getline(line_stream, cell, separator)) {
            T value;
            std::stringstream(cell) >> value;
            row.push_back(value);
        }

        v.push_back(row);
    }

    f.close();
}

}  // namespace iestaade
