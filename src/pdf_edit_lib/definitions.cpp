/* Copyright (C) 2021 Marco Scarpetta
 *
 * This file is part of PDF Mix Tool.
 *
 * PDF Mix Tool is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PDF Mix Tool is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PDF Mix Tool. If not, see <http://www.gnu.org/licenses/>.
 */

#include "definitions.h"

bool parse_output_pages_string(const std::string &str,
                               int n_pages,
                               std::vector<std::pair<int, int>> &intervals,
                               int &output_pages_count)
{
    intervals.clear();
    output_pages_count = 0;

    // Invalid characters
    if (str.find_first_not_of("0123456789- ,") != std::string::npos)
        return false;

    // Void string
    if (str.find_first_not_of("- ,") == std::string::npos)
    {
        output_pages_count = n_pages;
        intervals.push_back(std::pair<int, int>(0, n_pages - 1));
        return true;
    }

    // Parse string
    std::string::size_type cursor = str.find_first_not_of(" ,-");
    std::string::size_type interval_end = str.find_first_of(" ,", cursor);

    while (cursor < str.length())
    {
        // Single number
        if (str.find_first_of('-', cursor) >= interval_end)
        {
            std::string page_number = str.substr(cursor, interval_end - cursor);
            int num = std::stoi(page_number) - 1;

            // Syntax error
            if (num < 0 || num > n_pages - 1)
                return false;

            intervals.push_back(std::pair<int, int>(num, num));
            output_pages_count++;

            cursor = str.find_first_not_of(" ,-", interval_end);
            interval_end = str.find_first_of(" ,", cursor);
        }
        // Interval
        else
        {
            std::string::size_type first_number_end =
                    str.find_first_of('-', cursor);
            std::string::size_type second_number_start =
                    str.find_first_not_of('-', first_number_end);
            if (
                    // Syntax error: no second number
                    second_number_start >= interval_end ||
                    // Syntax error: more '-' in one interval
                    str.find_first_of('-', second_number_start) < interval_end
                    )
                return false;

            int from = std::stoi(str.substr(cursor, first_number_end - cursor)) - 1;
            int to = std::stoi(str.substr(second_number_start, interval_end)) - 1;

            // Syntax error
            if (from > to || from < 0 || to > n_pages - 1)
                return false;

            intervals.push_back(std::pair<int, int>(from, to));
            output_pages_count += to - from + 1;

            cursor = str.find_first_not_of(" ,-", interval_end);
            interval_end = str.find_first_of(" ,", cursor);
        }
    }

    if (intervals.size() == 0)
        intervals.push_back(std::pair<int, int>(0, n_pages - 1));

    return true;
}
