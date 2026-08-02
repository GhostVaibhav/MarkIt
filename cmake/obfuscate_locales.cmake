# Variables required:
# LOCALES_DIR: path to locales directory
# OUTPUT_FILE: path to generated header file

file(GLOB json_files "${LOCALES_DIR}/*.json")
set(header_content "#pragma once\n#include <string>\n#include <vector>\n\nnamespace ObfuscatedLocales {\n    inline std::string get(const std::string& langCode) {\n")

set(KEY 0x55)

foreach(file IN LISTS json_files)
    get_filename_component(lang_code "${file}" NAME_WE)
    file(READ "${file}" json_content HEX)
    string(REGEX MATCHALL "(..)" hex_chars "${json_content}")
    set(obfuscated "")
    foreach(hex_char IN LISTS hex_chars)
        math(EXPR obfuscated_char "0x${hex_char} ^ ${KEY}")
        string(APPEND obfuscated "${obfuscated_char}, ")
    endforeach()
    
    string(APPEND header_content "        if (langCode == \"${lang_code}\") {\n")
    string(APPEND header_content "            const unsigned char data[] = { ${obfuscated} };\n")
    string(APPEND header_content "            std::string result;\n")
    string(APPEND header_content "            for (auto c : data) result += (char)(c ^ ${KEY});\n")
    string(APPEND header_content "            return result;\n")
    string(APPEND header_content "        }\n")
endforeach()

string(APPEND header_content "        return \"\";\n")
string(APPEND header_content "    }\n}\n")

file(WRITE "${OUTPUT_FILE}" "${header_content}")
