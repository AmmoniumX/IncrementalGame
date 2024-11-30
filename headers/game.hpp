#pragma once
#include <boost/multiprecision/gmp.hpp>
#include "json.hpp"
#include <string>

typedef boost::multiprecision::mpz_int bigint;
using std::string;
using nlohmann::json;

typedef struct {
    bigint pps;
} GAME_DATA;

GAME_DATA DEFAULT_GAME_DATA = {
    pps: bigint(0)
};

json to_json(const GAME_DATA& data);
bool from_json(const json& j, GAME_DATA& data);
void save(GAME_DATA& data, string filename);
GAME_DATA load(string filename);