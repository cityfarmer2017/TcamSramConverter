/**
 * Copyright [2024] <wangdianchao@ehtcn.com>
 */
#ifndef MATCH_ACTION_ID_H_
#define MATCH_ACTION_ID_H_

#include <vector>
#include <string>
#include <map>

int convert_tcam_2_sram(const std::map<std::string, std::string>&);
int extract_action_ids(const std::map<std::string, std::string>&);

#endif  // MATCH_ACTION_ID_H_
