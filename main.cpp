#include <cstdio>
#include <cstdint>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <iostream>
#include <regex>
#include <fstream>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <vector>
#ifdef _WIN32
#include <windows.h>
#else
#include <limits.h>
#include <libgen.h>
#endif

std::string get_executable_directory() {
    std::string path;
    std::vector<char> buffer;
#ifdef _WIN32
    buffer.resize(MAX_PATH);
    GetModuleFileName(NULL, buffer.data(), buffer.size());
    path = std::string(buffer.data());
    path = path.substr(0, path.find_last_of("\\/") + 1);
#else
    buffer.resize(PATH_MAX);
    ssize_t len = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);
    if (len != -1) {
        buffer[len] = '\0';
        path = std::string(dirname(buffer.data()));
        path += "/";
    }
#endif
    return path;
}

struct Section {
    const char* name;
    uint32_t start;
    uint32_t end;
};

void unpack_sections(const char* input_filename, const std::string& executable_directory, const std::string& output_folder_param) {

    std::string input_path(input_filename);
    std::size_t found = input_path.find_last_of("/\\");
    std::string base_path = input_path.substr(0, found + 1);

    std::ifstream input(input_filename, std::ios::binary);
    if (!input) {
        std::cerr << "Error opening input file." << std::endl;
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        return;
    }

    // Check the length of the input file.
    input.seekg(0, std::ios::end);
    std::streamoff size = input.tellg();
    if (size != 0x110000) {
        std::cerr << "Error: Input file length is " << size << ", expected 0x110000." << std::endl;
        input.close();
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        return;
    }
    input.seekg(0, std::ios::beg);

    Section sections[] = {
        {"webkit_vsh_gadgets_1", 0x00000000, 0x0006FFFF},
        {"sprx", 0x00070000, 0x0007FFF7},
        {"size_of_stage2", 0x0007FFF8, 0x0007FFFF},
        {"stage2", 0x00080000, 0x0009FFF7},
        {"webkit_vsh_gadgets_2", 0x0009FFF8, 0x000A0387},
        {"padding1", 0x000A0388, 0x000A4FAF},
        {"socket_lv2_value", 0x000A4FB0, 0x000A5EEF},
        {"padding2", 0x000A5EF0, 0x000FFFDF},
        {"stackframe_lv2_vsh_offsets", 0x000FFFE0, 0x00101FFF},
        {"stage0", 0x00102000, 0x0010EFFF},
        {"padding3", 0x0010F000, 0x0010FFFF}
    };


    unsigned char* buffer = new unsigned char[static_cast<unsigned int>(size)];
    input.read(reinterpret_cast<char*>(buffer), size);
    input.close();


    // Ask for firmware version
	std::string output_folder;
    std::regex fw_regex("^\\d{3}[cCdD]$");

    if (output_folder_param.empty()) {
        std::string firmware_version;
        std::cout << "Enter firmware version (e.g. 490C) or leave blank for current directory: ";
        std::getline(std::cin, firmware_version);
        if (!firmware_version.empty() && std::regex_match(firmware_version, fw_regex)) {
            output_folder = executable_directory.empty() ? "./" : executable_directory;
            output_folder += firmware_version + "/";
        } else {
            output_folder = executable_directory.empty() ? "./" : executable_directory;
        }
    } else {
        output_folder = executable_directory.empty() ? "./" : executable_directory;
        output_folder += output_folder_param + "/";
    }

    #ifdef _WIN32
        _mkdir(output_folder.c_str());
    #else
        mkdir(output_folder.c_str(), 0755);
    #endif

    for (const auto& section : sections) {
        std::ostringstream filename_stream;
		filename_stream << std::setfill('0') << std::setw(2) << (int)(&section - sections) + 1 << "_0x" << std::uppercase << std::hex << std::setw(8) << section.start << "-0x" << std::setw(8) << section.end << "_" << section.name << ".bin";


        std::string filename = filename_stream.str();
        std::string filepath = output_folder + filename;

        std::ofstream output(filepath, std::ios::binary);
        if (!output) {
            std::cerr << "Error opening output file: " << filepath << std::endl;
            return;
        }

        output.write(reinterpret_cast<char*>(buffer + section.start), section.end - section.start + 1);
        output.close();

        std::cout << "Created file: " << filepath << std::endl;
    }

    delete[] buffer;

    // Pause after unpack_sections is done
    //std::cout << "\nUnpack operation completed. Press Enter to continue..." << std::endl;
    //std::cin.get();
}

bool file_exists(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

void pack_sections(const char* output_filename, Section* sections, int section_count) {
    if (file_exists(output_filename)) {
        std::cerr << "Error: Output file " << output_filename << " already exists. Please choose another filename." << std::endl;
        std::cout << "\nPress Enter to exit..." << std::endl;
        std::cin.get();
        return;
    }

    std::ofstream output(output_filename, std::ios::binary);
    if (!output) {
        std::cerr << "Error opening output file." << std::endl;
        return;
    }

    std::vector<unsigned char> buffer(0x110000);
    std::fill(buffer.begin(), buffer.end(), 0);

    bool error_found = false;

    for (int i = 0; i < section_count; i++) {
        const auto& section = sections[i];
        std::ostringstream filename_stream;
        filename_stream << "0x" << std::hex << section.start << "-0x" << section.end << "_" << section.name << ".bin";
        std::string filename = filename_stream.str();

        struct stat st;
        if (stat(filename.c_str(), &st) != 0) {
            std::cerr << "Error: Section file " << filename << " is missing." << std::endl;
            error_found = true;
            continue;
        }

        uint32_t expected_size = section.end - section.start + 1;
        if (st.st_size != static_cast<int64_t>(expected_size)) { // Cast to int64_t
            std::cerr << "Error: Section file " << filename << " has a size of 0x" << std::hex << st.st_size << ", expected 0x" << expected_size << "." << std::endl;
            error_found = true;
            continue;
        }

        std::ifstream input(filename, std::ios::binary);
        if (!input) {
            std::cerr << "Error opening input file: " << filename << std::endl;
            error_found = true;
            continue;
        }

        input.read(reinterpret_cast<char*>(buffer.data() + section.start), section.end - section.start + 1);
        input.close();
    }

    if (error_found) {
        std::cerr << "Errors found. Exiting without creating a new PS3HEN.BIN file." << std::endl;
        output.close();
        return;
    }

    output.write(reinterpret_cast<char*>(buffer.data()), 0x110000);
    output.close();
}


void print_help(const char* exe_name) {
    printf("Usage: %s /unpack|/pack PS3HEN.BIN\n", exe_name);
    printf("Commands:\n");
    printf("  /unpack PS3HEN.BIN      Unpack the PS3HEN.BIN file into separate sections\n");
    printf("  /pack PS3HEN_NEW.BIN     Pack the separate section files back into a new file\n");
}

int main(int argc, char* argv[]) {
    std::string executable_directory = get_executable_directory();

    if (argc < 2) {
        print_help(argv[0]);

        // Pause and wait for the user to press Enter
        printf("\nPress Enter to exit...\n");
        getchar();

        return 1;
    }

    Section sections[] = {
        {"webkit_vsh_gadgets_1", 0x00000000, 0x0006FFFF},
        {"sprx", 0x00070000, 0x0007FFF7},
        {"size_of_stage2", 0x0007FFF8, 0x0007FFFF},
        {"stage2", 0x00080000, 0x0009FFF7},
        {"webkit_vsh_gadgets_2", 0x0009FFF8, 0x000A0387},
        {"padding1", 0x000A0388, 0x000A4FAF},
        {"socket_lv2_value", 0x000A4FB0, 0x000A5EEF},
        {"padding2", 0x000A5EF0, 0x000FFFDF},
        {"stackframe_lv2_vsh_offsets", 0x000FFFE0, 0x00101FFF},
        {"stage0", 0x00102000, 0x0010EFFF},
        {"padding3", 0x0010F000, 0x0010FFFF}
    };

    const char* command;
    const char* filename;
    std::string output_folder;

    if (argc == 2) {
        // Treat a dropped file as an automatic "/unpack" command
        command = "/unpack";
        filename = argv[1];
    } else {
        command = argv[1];
        filename = argv[2];

        if (argc > 3) {
            if (strcmp(argv[3], "/out") == 0) {
                if (argc > 4) {
                    output_folder = argv[4];
                } else {
                    fprintf(stderr, "Missing output path after /out switch.\n");
                    return 1;
                }
            }
        }
    }

    if (strcmp(command, "/unpack") == 0) {
        unpack_sections(filename, executable_directory, output_folder);
    } else if (strcmp(command, "/pack") == 0) {
        pack_sections(filename, sections, sizeof(sections) / sizeof(sections[0]));
    } else {
        fprintf(stderr, "Invalid command: %s\n", command);
        return 1;
    }

    return 0;
}


