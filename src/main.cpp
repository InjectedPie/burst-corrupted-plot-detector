/* Burst Corrupted Plot Detector - Scans Burst mining logs and reports 
 * possible corrupt plot files.
 * Copyright(C) 2018 Guney Ozsan
 *
 * This program is free software : you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>

#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include "dirent.h"
#else
#include <dirent.h>
#endif // _MSC_VER

#include "console_gui.h"
#include "file_utility.h"

struct Plot_file {
	std::string name;
	struct Stats {
		int healthy_count = 0;
		int corrupted_count = 0;
	};
	Stats stats;
	Plot_file() {}
	Plot_file(std::string name){
		this->name = name;
	}
};

static std::vector<Plot_file> find_corrupted_plots(const char *file_name);
static void print_plot_file_stats(std::vector<Plot_file> plot_file_result);

int main(int argc, char *argv[]) {
	int i;
	std::vector<dirent> files_in_dir;

	/* For each directory in command line */
	i = 1;
	while (i < argc) {
		files_in_dir = get_files_in_directory(argv[i]);
		i++;
	}

	/* List current working directory if no arguments on command line */
	if (argc == 1) {
		files_in_dir = get_files_in_directory(".");
	}

	for (int i = 0; i < files_in_dir.size(); i++) {
		std::cout << std::endl;
		std::cout << "CHECKING FILE -> " << files_in_dir[i].d_name << std::endl;

		std::vector<Plot_file> plot_files;
		plot_files = find_corrupted_plots(files_in_dir[i].d_name);
		print_plot_file_stats(plot_files);
	}

	return EXIT_SUCCESS;
}

/*
* Find Burst plots with deadlines different from server's deadline.
*/
static std::vector<Plot_file> find_corrupted_plots(const char *file_name) {
	std::string found_deadline;
	std::string confirmed_deadline;
	std::string plot_file_name;
	std::map<std::string, Plot_file> plot_files;
	const std::string found_deadline_keyword = "found deadline=";
	const std::string found_deadline_end_keyword = " nonce";
	const std::string confirmed_deadline_keyword = "confirmed deadline: ";
	const std::string file_keyword = "file: ";
	std::size_t position = 0;
	std::size_t end_position = 0;
	std::size_t start_position = 0;
	std::size_t plot_file_position = 0;

	std::ifstream file(file_name);
	std::string line;

	std::cout << "DEADLINES -> ";

	std::string busy_icon[] = { "\'", "\'", ":", ".", ":" };

	size_t busy_icon_animation_length = sizeof(busy_icon) / sizeof(busy_icon[0]);
	float cursor_time = 0;
	float update_speed = 0.002f;
	int last_update = 0;
	std::string current_frame = busy_icon[0];
	std::cout << whitespace(current_frame.length());
	while (std::getline(file, line))
	{
		// Print busy icon
		if (cursor_time > last_update) {
			current_frame = busy_icon[(int)cursor_time % busy_icon_animation_length];
			std::cout << backspace(current_frame.length()) << current_frame;
			last_update++;
		}
		cursor_time += update_speed;

		// Extract found deadlines.
		position = line.find(found_deadline_keyword, position + 1);
		if (position != std::string::npos) {
			end_position = line.find(found_deadline_end_keyword, position + 1);
			start_position = position + found_deadline_keyword.size();
			found_deadline = line.substr(start_position, end_position - start_position);
			// Extract file name.
			plot_file_position = line.find(file_keyword, end_position + found_deadline_end_keyword.size()) + file_keyword.size();
			plot_file_name = line.substr(plot_file_position, line.size());
			if (plot_files.count(plot_file_name) == 0) {
				plot_files[plot_file_name] = Plot_file(plot_file_name);
			}
		}

		// Extract confirmed deadlines.
		position = line.find(confirmed_deadline_keyword, position + 1);
		if (position != std::string::npos) {
			end_position = line.size();
			start_position = position + confirmed_deadline_keyword.size();
			confirmed_deadline = line.substr(start_position, end_position - start_position);
		}

		// Corrupted plot condition:
		if (found_deadline != "" && confirmed_deadline != "") {
			if (found_deadline == confirmed_deadline) {
				plot_files[plot_file_name].stats.healthy_count++;
				std::cout << backspace(current_frame.length()) << "." << busy_icon[(int)cursor_time % busy_icon_animation_length];
			}
			else {
				plot_files[plot_file_name].stats.corrupted_count++;
				std::cout << backspace(current_frame.length()) << "X" << busy_icon[(int)cursor_time % busy_icon_animation_length];
			}
			found_deadline = "";
			confirmed_deadline = "";
		}
	}
	std::cout << backspace(current_frame.length()) << whitespace(current_frame.length());
	std::vector<Plot_file> plot_files_array(plot_files.size());
	int i = 0;
	for (auto const &it : plot_files) {
		plot_files_array[i] = it.second;
		i++;
	}
	return plot_files_array;
}

/*
* Display the stats of the given plot file in a nice format.
*/
static void print_plot_file_stats(std::vector<Plot_file> plot_files) {
	const std::string corrupted_title = "CONFLICTING";
	const std::string healthy_title = "HEALTHY";
	const std::string plot_file_title = "PLOT FILE";
	const std::string title_gap = "   ";

	if (plot_files.size() > 0) {
		std::cout << std::endl;
		std::cout << corrupted_title << title_gap << healthy_title << title_gap << plot_file_title << std::endl;
		std::cout << underline(corrupted_title) << title_gap << underline(healthy_title) << title_gap << underline(plot_file_title) << std::endl;
		std::string corrupted_count;
		std::string healthy_count;
		for (size_t i = 0; i < plot_files.size(); i++) {
			if (plot_files[i].stats.corrupted_count == 0) {
				corrupted_count = "-";
			}
			else {
				corrupted_count = std::to_string(plot_files[i].stats.corrupted_count);
			}

			print_right_aligned(corrupted_count, corrupted_title.length());
			std::cout << title_gap;

			if (plot_files[i].stats.healthy_count == 0) {
				healthy_count = "-";
			}
			else {
				healthy_count = std::to_string(plot_files[i].stats.healthy_count);
			}

			print_right_aligned(healthy_count, healthy_title.length());
			std::cout << title_gap;
			std::cout << plot_files[i].name;
			std::cout << std::endl;
		}
	}
	else {
		std::cout << "No deadlines detected." << std::endl;
	}
}