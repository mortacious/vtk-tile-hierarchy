//
// Created by mortacious on 12/16/21.
//

#pragma once
#include <istream>
#include <memory>

std::unique_ptr<std::istream> DownloadFile(const std::string& url);