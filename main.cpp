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

// TODO: check some bytes from input file to verify its a PS3HEN.BIN file

#ifdef _WIN32
    const char PATH_SEPARATOR = '\\';
#else
    const char PATH_SEPARATOR = '/';
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

void unpack_sections(const char* input_filename, const std::string& executable_directory, const std::string& output_directory_param) {

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

    unsigned char* buffer = new unsigned char[static_cast<unsigned int>(size)];
    input.read(reinterpret_cast<char*>(buffer), size);
    input.close();


    // Ask for firmware version
	std::string output_directory;
    std::regex fw_regex("^\\d{3}[cCdD]$");

    if (output_directory_param.empty()) {
        std::string firmware_version;
        std::cout << "Enter firmware version (e.g. 490C) or leave blank for current directory: ";
        std::getline(std::cin, firmware_version);
        if (!firmware_version.empty() && std::regex_match(firmware_version, fw_regex)) {
            output_directory = executable_directory.empty() ? "./" : executable_directory;
            output_directory += firmware_version + "/";
        } else {
            output_directory = executable_directory.empty() ? "./" : executable_directory;
        }
    } else {
        output_directory = executable_directory.empty() ? "./" : executable_directory;
        output_directory += output_directory_param + "/";
    }

    #ifdef _WIN32
        _mkdir(output_directory.c_str());
    #else
        mkdir(output_directory.c_str(), 0755);
    #endif

	int section_count = 0; // Add a counter for the number of sections

    for (const auto& section : sections) {
        std::ostringstream filename_stream;
		filename_stream << std::setfill('0') << std::setw(2) << (int)(&section - sections) + 1 << "_0x" << std::uppercase << std::hex << std::setw(8) << section.start << "-0x" << std::setw(8) << section.end << "_" << section.name << ".bin";
        std::string filename = filename_stream.str();
        std::string filepath = output_directory + filename;

        std::ofstream output(filepath, std::ios::binary);
        if (!output) {
            std::cerr << "Error opening output file: " << filepath << std::endl;
            return;
        }

        output.write(reinterpret_cast<char*>(buffer + section.start), section.end - section.start + 1);
        output.close();

        std::cout << "Created file: " << filepath << std::endl;
        section_count++; // Increment the counter for each section created
    }
	
	std::cout << "Total number of sections unpacked: " << section_count << std::endl; // Print the total number of sections

    delete[] buffer;

    // Pause after unpack_sections is done
    //std::cout << "\nUnpack operation completed. Press Enter to continue..." << std::endl;
    //std::cin.get();
}

bool file_exists(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

void pack_sections(const char* output_filename, Section* sections, int section_count, const std::string& input_directory) {
	
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
        filename_stream << std::setfill('0') << std::setw(2) << (int)(&section - sections) + 1 << "_0x" << std::uppercase << std::hex << std::setw(8) << section.start << "-0x" << std::setw(8) << section.end << "_" << section.name << ".bin";
        std::string filename = input_directory + filename_stream.str();

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
        remove(output_filename); // Remove the zero-byte file if any errors were encountered
        return;
    }

    output.write(reinterpret_cast<char*>(buffer.data()), 0x110000);
    output.close();
}

void print_help() {
    printf("HEN Splitter v1.0\n");
    printf("esc0rtd3w / PS3Xploit Team 2023\n");
    printf("http://www.ps3xploit.me\n\n");
    printf("Usage: hen_splitter.exe /unpack|/pack PS3HEN.BIN /out|/in [directory]\n\n");
    printf("Drag and Drop is Supported\n\n");
    printf("Commands:\n");
    printf("  /unpack               Unpack the PS3HEN.BIN file into separate sections\n");
    printf("  /out                  Specify output directory for PS3HEN.BIN\n\n");
    printf("  /pack                 Pack the section files back into a new file\n");
    printf("  /in                   Specify input directory for sections\n\n");
    printf("  Unpack Example 1:     hen_splitter.exe /unpack PS3HEN.BIN\n");
    printf("  Unpack Example 2:     hen_splitter.exe /unpack PS3HEN.BIN /out \"490C\"\n\n");
    printf("  Pack Example 1:       hen_splitter.exe /pack PS3HEN_NEW.BIN\n");
    printf("  Pack Example 2:       hen_splitter.exe /pack PS3HEN_NEW.BIN /in \"490C\"\n\n");
}

int main(int argc, char* argv[]) {

    std::string executable_directory = get_executable_directory();
    std::string input_directory = "";
    std::string output_directory;
    const char* command;
    const char* filename;

    if (argc < 2) {
        print_help();

        // Pause and wait for the user to press Enter
        printf("\nPress Enter to exit...\n");
        getchar();

        return 1;
    }

    if (argc == 2) {
        // Treat a dropped file as an automatic "/unpack" command
        command = "/unpack";
        filename = argv[1];
    } else {
        command = argv[1];
        filename = argv[2];

        int i = 3;
        while (i < argc) {
            if (strcmp(argv[i], "/out") == 0) {
                if (i + 1 < argc) {
                    output_directory = argv[i + 1];
                    i += 2;
                } else {
                    fprintf(stderr, "Missing output path after /out switch.\n");
                    return 1;
                }
            } else if (strcmp(argv[i], "/in") == 0) {
                if (i + 1 < argc) {
                    input_directory = argv[i + 1];
                    // Add the following line to append the file separator
                    input_directory += PATH_SEPARATOR;
                    i += 2;
                } else {
                    fprintf(stderr, "Missing input path after /in switch.\n");
                    return 1;
                }
            } else {
                i++;
            }
        }
    }

    std::string original_executable_directory;

    if (strcmp(command, "/unpack") == 0) {
        unpack_sections(filename, executable_directory, output_directory);
    } else if (strcmp(command, "/pack") == 0) {
        if (!input_directory.empty()) {
            original_executable_directory = executable_directory;
            executable_directory += input_directory + "/";
        }
        pack_sections(filename, sections, sizeof(sections) / sizeof(sections[0]), input_directory);
        if (!input_directory.empty()) {
            executable_directory = original_executable_directory;
        }
    } else {
        fprintf(stderr, "Invalid command: %s\n", command);
        return 1;
    }
    
    return 0;
}

