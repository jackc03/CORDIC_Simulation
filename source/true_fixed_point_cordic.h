#ifndef includes
#define includes 
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <stdint.h>
#include <filesystem>
#include <sstream>
#include <vector>
#include <map>
#include <numbers>

#endif

#include "fixed_point.h"
typedef uint32_t uint32;


class instruction {
    public:
        // Which function to run, 1 for rotation mode; 2 for vector mode
        int funct;
        // X coord
        float x;
        // Y coord
        float y;
        // Angle in radians to rotate to for rotation mode
        float z;

        fixed_30 x_coord, y_coord;;

        instruction(std::vector<float> arr) {
            funct = arr[0];
            x     = arr[1];
            y     = arr[2];
            z     = arr[3];

        }
};



instruction* parseLine(const std::string& line) {
    std::istringstream stream(line);
    std::string token;
    std::map<std::string, std::string> data;
    std::vector<float> arr;
    int i = 0;

    // Split by commas
    while (std::getline(stream, token, ',')) {
        std::istringstream pairStream(token);
        std::string key, value;
        size_t size;

        
        // Split by equal sign (=)
        if (std::getline(pairStream, key, '=') && std::getline(pairStream, value)) {
            key.erase(0, key.find_first_not_of(" \t\n\r")); // Remove leading spaces
            data[key] = value;
            if (value == "rot")
                arr.push_back(1);
            else if( value == "vec")
                arr.push_back(2);     
            else
                arr.push_back(std::stof(value));
        }
    }

    // Output the extracted data
    instruction *a = new instruction(arr);

    // std::cout << a->funct << ", " << a->x << ", " << a->y << ", " << a->z << ", " << std::endl;

    return a;
}



std::string get_filename(const std::string& base_name) {
    std::string new_name, curr;
    // check if the name hasn't been used before, if not no need to increment
    if (!std::filesystem::exists(base_name)) {
        return base_name;
    }
    
    //we know it exists, now we need to see how many copies are in this dir
    std::string file_name = std::filesystem::path(base_name).stem().string();
    // Loop to check if previous outputs exist
    int i = 1;
    do {
        std::ostringstream output_str;
        output_str << "output/" << file_name << "_" << i << ".txt";
        // std::cout << file_name << "_" << i << ".txt" << std::endl;

        new_name = output_str.str();
        i++;
    } while (std::filesystem::exists(new_name));

    return new_name;
}

void print_angles(int NUM_ROTATIONS) {
    float k = 1;
    float angle;
    float sum = 0;
    float tan;
    // First precompute the scale factor, angle, and tangents
    for (int i = 0; i < NUM_ROTATIONS; ++i) {
        tan = std::pow(2, -1 * i);
        angle = std::atan(tan);
        k *= std::cos(angle);
        sum += angle;

        std::cout << "angle = " << angle  << ", tan(angle) = " << tan << std::endl;
    }
    std::cout << sum << std::endl;
    // k = 1.0/k;
    std::cout << "k = " << k << std::endl;
}

