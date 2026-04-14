#include <Functions.h>
#include <Class_Image.h>
#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>

using namespace std;

int main()
{   
    std::filesystem::path current_directory = std::filesystem::current_path();
    std::cout << "Current working directory: " << current_directory << std::endl;
    
    std::cout << "The working directory has been successfully identified!" << std::endl;

    int count_of_pixels_k = 16800;
    string load_from_filename = "../../../../resources/input/" + to_string(count_of_pixels_k) +"k.bmp";
    string load_to_filename = "../../../../resources/output/" + to_string(count_of_pixels_k) + "k.bmp";

    std::cout << "The path to the resource file has been successfully collected!" << std::endl;

    int height, width;
    std::vector<uint8_t> char_data;

    loadBMP(load_from_filename, char_data, width, height);

    std::vector<float> float_data = convertToFloat(char_data);
    
    Image image(width, height, std::move(float_data));
    
    Image result = Sobel_operator(image);
    
    // normalize(result.imData());
    char_data = convertToChar(result.imData());

    saveBMP(load_to_filename, char_data, result.imWidth(), result.imHeight());
}