#include "Todo.h"
#include "Logger.h"
#include "picosha2.h"
#include <ctime>

Todo Todo::create(const std::string& name, const std::string& desc) {
    Todo t;
    t.name = name;
    t.desc = desc;
    t.time = static_cast<int32_t>(std::time(nullptr));
    t.isComplete = false;
    
    std::string id_input = name + desc + std::to_string(t.time);
    std::vector<unsigned char> hash(picosha2::k_digest_size);
    picosha2::hash256(id_input.begin(), id_input.end(), hash.begin(), hash.end());

    t.id = picosha2::bytes_to_hex_string(hash.begin(), hash.end());

    Logger::getInstance();
    spdlog::info("Todo created: " + t.name + ", " + t.desc + ", " + std::to_string(t.time) + ", " + std::to_string(t.isComplete) + ", " + t.id);
    
    return t;
}
