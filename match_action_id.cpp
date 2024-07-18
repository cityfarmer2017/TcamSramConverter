/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#include <unordered_map>
#include <fstream>
#include <regex>  // NOLINT [build/include_subdir]
#include <iostream>
#include <filesystem>
#include "match_action_id.h"  // NOLINT [build/include_subdir]

constexpr auto MAX_ACTION_ID = 256 * 32 - 1;

const std::unordered_map<std::string, std::string> c2_c4_map = {
    {"00", "1000"},
    {"01", "0100"},
    {"10", "0010"},
    {"11", "0001"},
    {"0x", "1100"},
    {"x0", "1010"},
    {"1x", "0011"},
    {"x1", "0101"},
    {"xx", "1111"}
};

int convert_tcam_2_sram(const std::map<std::string, std::string> &stem_parent_map) {
    std::vector<std::vector<std::string>> sram_chip_vec{20};
    std::size_t n = 0;
    for (const auto &stem_parent : stem_parent_map) {
        auto path = std::filesystem::path(stem_parent.second + "/" + stem_parent.first + ".tcam");
        std::ifstream ifstr(path);
        if (!ifstr.is_open()) {
            std::cout << "cannot open source file: " << path.string() << std::endl;
            return -1;
        }

        for (auto &sram_entry_vec : sram_chip_vec) {
            sram_entry_vec.emplace_back("");
            sram_entry_vec.emplace_back("");
            sram_entry_vec.emplace_back("");
            sram_entry_vec.emplace_back("");
        }

        std::string line;
        std::size_t sz = 0;
        while (getline(ifstr, line)) {
            const std::regex r(R"([01x]{40})");
            if (!regex_match(line, r)) {
                std::cout << "tcam entry shall only include 40 collums of [01x]." << std::endl;
                return -1;
            }

            std::vector<std::string> tcam_entry_vec;
            for (auto i = 0UL; i < line.size(); ++i, ++i) {
                tcam_entry_vec.emplace_back(line.substr(i, 2));
            }

            std::reverse(tcam_entry_vec.begin(), tcam_entry_vec.end());

            auto i = 0UL;
            for (const auto &c2 : tcam_entry_vec) {
                auto c4 = c2_c4_map.at(c2);
                sram_chip_vec[i][n*4+0].insert(0, 1, c4[0]);
                sram_chip_vec[i][n*4+1].insert(0, 1, c4[1]);
                sram_chip_vec[i][n*4+2].insert(0, 1, c4[2]);
                sram_chip_vec[i][n*4+3].insert(0, 1, c4[3]);
                ++i;
            }

            ++sz;
        }

        if (sz != 32) {
            std::cout << "a tcam table must be exactly 32 entries." << std::endl;
            return -1;
        }

        ++n;
    }

    auto dst_fname = stem_parent_map.cbegin()->second + "/" + stem_parent_map.cbegin()->first;
    std::ofstream ofstr;

    std::vector<std::uint32_t> u32_vec;
    ofstr.open(dst_fname + ".sram.txt");
    if (!ofstr.is_open()) {
        std::cout << "cannot open dest file: " << dst_fname + ".sram.txt" << std::endl;
        return -1;
    }
    auto i = 0UL;
    for (const auto &sram_entry_vec : sram_chip_vec) {
        ofstr << "//sram chip #" << std::setw(2) << std::setfill('0') << i << "\n";
        for (const auto &entry_str : sram_entry_vec) {
            ofstr << entry_str << "\n";
            u32_vec.emplace_back(std::stoul(entry_str, nullptr, 2));
            #ifdef DEBUG
            std::cout << entry_str << "\n";
            std::cout << std::stoul(entry_str, nullptr, 2) << "\n";
            #endif
        }
        ++i;
    }
    ofstr << std::flush;
    ofstr.close();

    ofstr.open(dst_fname + ".sram.dat", std::ios::binary);
    if (!ofstr.is_open()) {
        std::cout << "cannot open dest file: " << dst_fname + ".sram.dat" << std::endl;
        return -1;
    }
    ofstr.write(reinterpret_cast<const char*>(u32_vec.data()), sizeof(u32_vec[0]) * u32_vec.size());
    ofstr.close();

    return 0;
}

int extract_action_ids(const std::map<std::string, std::string> &stem_parent_map) {
    std::vector<std::uint16_t> u16_vec;

    for (const auto &stem_parent : stem_parent_map) {
        auto path = std::filesystem::path(stem_parent.second + "/" + stem_parent.first + ".aid");
        std::ifstream ifstr(path);
        if (!ifstr.is_open()) {
            std::cout << "cannot open source file: " << path.string() << std::endl;
            return -1;
        }

        std::string line;
        std::size_t sz = 0;
        while (getline(ifstr, line)) {
            const std::regex r(R"(([01]{16})|\d{1,5}|0[xX][\da-fA-F]{1,4})");
            std::smatch m;
            if (!regex_match(line, m, r)) {
                std::cout << "action_id entry shall only be [01] string, decaimal or heximal of 16 bits." << std::endl;
                return -1;
            }

            auto val = 0UL;
            if (m.str(1).empty()) {
                val = std::stoul(m.str(), nullptr, 0);
            } else {
                val = std::stoul(m.str(), nullptr, 2);
            }
            if (val <= MAX_ACTION_ID) {
                u16_vec.emplace_back(val);
            } else {
                std::cout << "an action id may not exceed " << MAX_ACTION_ID << std::endl;
                return -1;
            }

            ++sz;
        }

        if (sz != 32) {
            std::cout << "an action_id table must be exactly 32 entries." << std::endl;
            return -1;
        }
    }

    auto dst_fname = stem_parent_map.cbegin()->second + "/" + stem_parent_map.cbegin()->first + ".aid.dat";
    std::ofstream ofstr(dst_fname, std::ios::binary);
    if (!ofstr.is_open()) {
        std::cout << "cannot open dest file: " << dst_fname << std::endl;
        return -1;
    }
    ofstr.write(reinterpret_cast<const char*>(u16_vec.data()), sizeof(u16_vec[0]) * u16_vec.size());
    ofstr.close();

    return 0;
}
