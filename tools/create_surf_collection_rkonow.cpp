// extracts from an indri index a monoton sequence of integers in sdsl format
// which represent the parsed text collection.
#include <iostream>
#include <dirent.h>
#include <fstream>
#include <streambuf>
#include "surf/config.hpp"
#include "surf/util.hpp"
#include "sdsl/int_vector_buffer.hpp"

void list_files(const std::string& directory, std::vector<std::string>& files) {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (directory.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            files.push_back(ent->d_name);
        }
        closedir (dir);
    } else {
        perror ("Could not open directory");
    }
}

std::string read_file(const std::string& file) {
    std::string output;
    std::ifstream in(file.c_str());
    in.seekg(0, std::ios::end);
    if (in.tellg() > 0 ) {
        output.reserve(in.tellg());
        in.seekg(0, std::ios::beg);
        output.assign((std::istreambuf_iterator<char>(in)),
                std::istreambuf_iterator<char>());
    } else {
        std::cout << "file is 0?" << file  << std::endl;
    }
    return output;
}

int main( int argc, char** argv ) {
    if(argc != 3) {
        std::cout << "USAGE: " << argv[0]
                  << " <folder with documents> <surf collection folder>" << std::endl;
        return EXIT_FAILURE;
    }
    const std::string folder = argv[1];
    const std::string dir = argv[2];

    if(!surf::directory_exists(folder)) {
        std::cerr << "ERROR:  directory containing documents doesn't exists." << std::endl;
        return EXIT_FAILURE;
    }

    // setup collection directory
    if(surf::directory_exists(dir)) {
        std::cerr << "ERROR: collection directory already exists." << std::endl;
        return EXIT_FAILURE;
    }
    surf::create_directory(dir);

    std::vector<std::string> files;
    list_files(folder, files);
    // write collection string
    {
        sdsl::int_vector_buffer<8> text_collection("temp.sdsl",std::ios::out);
        for (const auto& f : files) {
            std::string data = read_file(folder+"/"+f);
            if (data.size() > 0) {
                for (const auto &sym : data) {
                    text_collection.push_back(static_cast<char>(sym));
                }
                text_collection.push_back(1);
            }
        }
        text_collection.push_back(0);
    }
    sdsl::int_vector<8> text_col;
    sdsl::load_from_file(text_col, "temp.sdsl");
    sdsl::remove("temp.sdsl");
    std::ofstream ofs(dir+"/"+"text_SURF.sdsl");
    if(ofs.is_open()) {
        text_col.serialize(ofs);
    } else {
        std::cerr << "ERROR: could not write collection file." << std::endl;
        return EXIT_FAILURE;
    }
    ofs.close();
    std::cout << "Created surf collection for folder '" << folder << "'" << std::endl;
    std::cout << "Found " << files.size() << " documents" << std::endl;
}


