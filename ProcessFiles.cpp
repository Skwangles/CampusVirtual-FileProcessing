#include <iostream>
#include <cstdlib>     // For system()
#include <filesystem>  // For filesystem operations
#include <fstream>     // For file operations
#include <regex>       // For regex matching

namespace fs = std::filesystem;

#define OUTPUT_SIZE "1920x960"

// Function to run MediaSDKTest for each input path
void run_MediaSDKTest(const std::string& input_path, const std::string& output_dir, const std::string& second_eye = "", const std::string& filename = "") {
    // Check the file extension to determine how to process
    std::string file_extension = input_path.substr(input_path.find_last_of(".") + 1);

    if (file_extension == "insp") {
        // For .insp image files
        std::string command = "MediaSDKTest -inputs \"" + 
                input_path + "\" " + 
            "-output \"" + output_dir + "\" "+ 
            "-stitch_type dynamicstitch " +  
            "-image_sequence_dir \"" + output_dir + "\" "+
            "-image_type png"+ 
            "-output_size " + OUTPUT_SIZE;
        system(command.c_str());
    } else if (file_extension == "insv") {
        // For pairs of .insv video files
        std::string first_eye = input_path;
        std::string command = "MediaSDKTest -inputs \"" 
                    + first_eye + "\" \"" 
                    + second_eye + "\" " + 
                "-output \"" + output_dir + "/" + filename  + ".mp4" + "\" "+ 
                "-stitch_type dynamicstitch " + 
                "-output_size " + OUTPUT_SIZE;
        system(command.c_str());
    } else {
        std::cerr << "Unsupported file extension: " << file_extension << std::endl;
    }
}

// Function to copy JSON files from input directory to output directory
void copy_json_files(const std::string& input_dir, const std::string& output_dir) {
    // Check if the input directory exists
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Input directory '" << input_dir << "' not found." << std::endl;
        return;
    }

    // Check if the output directory exists, create if not
    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }

    // Iterate over JSON files in input directory and copy to output directory
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            fs::copy_file(entry.path(), output_dir + "/" + entry.path().filename().string(), fs::copy_options::overwrite_existing);
        }
    }

    std::cout << "Successfully copied JSON files from '" << input_dir << "' to '" << output_dir << "'." << std::endl;
}

// Function to embed metadata from JSON file into media files
void embed_metadata(const std::string& json_dir, const std::string& output_dir) {
    // Iterate over files in the output directory
    for (const auto& entry : fs::directory_iterator(output_dir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            std::string file_extension = entry.path().extension();

            // Extract filename without extension
            std::string filename_no_ext = filename.substr(0, filename.find_last_of("."));

            // Check if there's a corresponding .json file
            if (fs::exists(json_dir + "/" + filename_no_ext + ".json")) {
                std::ifstream json_file(json_dir + "/" + filename_no_ext + ".json");
                if (json_file.is_open()) {
                    std::string json_content((std::istreambuf_iterator<char>(json_file)), std::istreambuf_iterator<char>());
                    json_file.close();

                    // Embedding metadata based on file extension
                    if (file_extension == ".jpg" || file_extension == ".jpeg" || file_extension == ".png") {
                        std::string command = "exiftool -TagsFromFile \"" + output_dir + "/" + filename_no_ext + ".json\" \"-group<all:all\" \"" + output_dir + "/" + filename + "\"";
                        system(command.c_str());
                    } else if (file_extension == ".mp4" || file_extension == ".avi" || file_extension == ".mov") {
                        std::string command = "exiftool -json=\"" + output_dir + "/" + filename_no_ext + ".json\" \"-group<all:all\" \"" + output_dir + "/" + filename + "\"";
                        system(command.c_str());
                    } else {
                        std::cerr << "Unsupported file format: " << filename << std::endl;
                    }
                } else {
                    std::cerr << "Failed to open JSON file: " << output_dir + "/" + filename_no_ext + ".json" << std::endl;
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    // Check if the correct number of arguments are provided
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <videos_or_images_directory> <json_input_directory> <output_directory>" << std::endl;
        return 1;
    }

    std::string input_dir = argv[1];
    std::string json_input_dir = argv[2];
    std::string output_dir = argv[3];

    // Check if the input directory exists
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Input directory '" << input_dir << "' not found." << std::endl;
        return 1;
    }

    // Loop through each file in the input directory
    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (entry.is_regular_file()) {
            std::string input_path = entry.path().string();
            std::string filename = entry.path().filename().string();
            std::string file_extension = entry.path().extension();

            // Determine the file extension and process accordingly
            if (file_extension == ".insp") {
                run_MediaSDKTest(input_path, output_dir);
            } else if (file_extension == ".insv") {
                // For pairs of .insv video files
                // Extract the second insv file path based on the naming convention    
                std::regex pattern("_00_[0-9]{3}\\.insv$");
                if (std::regex_search(input_path, pattern)) {
                    std::string second_eye = std::regex_replace(filename, std::regex("_00_"), "_10_");
                    std::string second_eye_path = input_dir + "/" + second_eye;
                    run_MediaSDKTest(input_path, output_dir, second_eye_path, filename);
                }
            } else {
                std::cerr << "Skipping unsupported file: " << input_path << std::endl;
            }
        }
    }

    // Embed metadata from JSON files into output files
    embed_metadata(json_input_dir, output_dir);

    return 0;
}

