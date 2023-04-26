#include <cstdio>
#include <cstdint>
#include <string>
#include <cstring>
#include <sys/stat.h>

struct Section {
    const char* name;
    uint32_t start;
    uint32_t end;
};

void unpack_sections(const char* input_filename) {
    FILE* input = fopen(input_filename, "rb");
    if (!input) {
        fprintf(stderr, "Error opening input file.\n");
        return;
    }

    // Check the length of the input file.
    fseek(input, 0, SEEK_END);
    long size = ftell(input);
    if (size != 0x110000) {
        fprintf(stderr, "Error: Input file length is %ld, expected 0x110000.\n", size);
        return;
    }
    fseek(input, 0, SEEK_SET);

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

    unsigned char* buffer = new unsigned char[size];
    fread(buffer, 1, size, input);
    fclose(input);

    for (const auto& section : sections) {
        char filename[256];
        sprintf(filename, "0x%08X-0x%08X_%s.bin", section.start, section.end, section.name);
        FILE* output = fopen(filename, "wb");
        if (!output) {
            fprintf(stderr, "Error opening output file: %s\n", filename);
            return;
        }

        fwrite(buffer + section.start, 1, section.end - section.start + 1, output);
        fclose(output);

        printf("Created file: %s\n", filename);
    }

    delete[] buffer;
}

bool file_exists(const char* filename) {
    struct stat st;
    return (stat(filename, &st) == 0);
}

void pack_sections(const char* output_filename, Section* sections, int section_count) {
	
    if (file_exists(output_filename)) {
        fprintf(stderr, "Error: Output file %s already exists. Please choose another filename.\n", output_filename);
        return;
    }

    FILE* output = fopen(output_filename, "wb");
    if (!output) {
        fprintf(stderr, "Error opening output file.\n");
        return;
    }

    unsigned char* buffer = new unsigned char[0x110000];
    memset(buffer, 0, 0x110000);

    bool error_found = false;

    for (int i = 0; i < section_count; i++) {
        const auto& section = sections[i];
        char filename[256];
        sprintf(filename, "0x%08X-0x%08X_%s.bin", section.start, section.end, section.name);

        struct stat st;
        if (stat(filename, &st) != 0) {
            fprintf(stderr, "Error: Section file %s is missing.\n", filename);
            error_found = true;
            continue;
        }

        uint32_t expected_size = section.end - section.start + 1;
        if (st.st_size != expected_size) {
            fprintf(stderr, "Error: Section file %s has a size of 0x%lX, expected 0x%X.\n", filename, st.st_size, expected_size);
            error_found = true;
            continue;
        }

        FILE* input = fopen(filename, "rb");
        if (!input) {
            fprintf(stderr, "Error opening input file: %s\n", filename);
            error_found = true;
            continue;
        }

        fread(buffer + section.start, 1, section.end - section.start + 1, input);
        fclose(input);
    }

    if (error_found) {
        fprintf(stderr, "Errors found. Exiting without creating a new PS3HEN.BIN file.\n");
        delete[] buffer;
        fclose(output);
        return;
    }

    fwrite(buffer, 1, 0x110000, output);
    fclose(output);

    delete[] buffer;
}

void print_help(const char* exe_name) {
    printf("Usage: %s /unpack|/pack PS3HEN.BIN\n", exe_name);
    printf("Commands:\n");
    printf("  /unpack PS3HEN.BIN      Unpack the PS3HEN.BIN file into separate sections\n");
    printf("  /pack PS3HEN_NEW.BIN     Pack the separate section files back into a new file\n");
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
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

    const char* command = argv[1];
    const char* filename = argv[2];

    if (strcmp(command, "/unpack") == 0) {
        unpack_sections(filename);
    } else if (strcmp(command, "/pack") == 0) {
        pack_sections(filename, sections, sizeof(sections) / sizeof(sections[0]));
    } else {
        fprintf(stderr, "Invalid command: %s\n", command);
        return 1;
    }

    return 0;
}
