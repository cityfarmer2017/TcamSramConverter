#ifndef UTILITY_H
#define UTILITY_H

#include <vector>
#include <string>
#include <filesystem>

int convert_tcam_2_sram(const std::vector<std::filesystem::path>&);
int extract_action_ids(const std::vector<std::filesystem::path>&);

#endif // UTILITY_H