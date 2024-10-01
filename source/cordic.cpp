#include "cordic.h"

#define NUM_ROATATIONS 12
const double angles[12] = {0.785398, 0.463648, 0.244979, 0.124355,
                          0.0624188, 0.0312398, 0.0156237, 0.00781234,
                          0.00390623, 0.00195312, 0.000976562, 0.000488281}; 





int main() {
    // Input file and a string to hold each line
    std::ifstream input_file;
    std::string line;

    // Array holding all instructions from input file
    std::vector<instruction> instr_arr;

    // Open the file and parse each line into an instruction
    input_file.open("../input/cordic_input.txt");
    while (std::getline(input_file, line)) {
        instr_arr.push_back(*parseLine(line));
    }



    // Find suitable name and open output file for writing to while processing     
    if (!std::filesystem::exists("../output")) {
        std::filesystem::create_directory("output");
    }
    std::string output_name = get_filename("output/cordic_output.txt");
    std::ofstream output_file(output_name);



    // Function that prints angle and corresponging tans and final output scale factor
    // not needed after I copied the values to a look up table
    // print_angles(NUM_ROATATIONS);



    // Helper variables for the cordic loop
    fixed_18 x_output, y_output;

    // Copies of x and y coord to do avoid corrupting info for eqns
    fixed_30 x_cpy, y_cpy;

    // padding so we dont lose nearly as much precision while bit shifting and adding, cranked it tf up
    int padding = 24;

    // Integer indicating whether we are doing a positive of negative rotation
    int direction;


    // Loop through input instructions and perform cordic algorithm
    for (int i = 0; i < instr_arr.size(); ++i) {
        instruction curr_instr = instr_arr[i];
            
        // Do implicit cast to fixed_30 with = operator overload
        curr_instr.x_coord = curr_instr.x;
        curr_instr.y_coord = curr_instr.y;
        std::cout << curr_instr.x_coord.to_float() << "  " << curr_instr.y_coord.to_float() << std::endl;

        // Rotation Mode
        if (curr_instr.funct == 1) {
            // Adjust for overflow if desired rotation is larger than is possible with given angles
            if (curr_instr.z > 1.742) {
                curr_instr.x *= -1.0;
                curr_instr.y *= -1.0;
                curr_instr.z -= M_PI;
            } else if (curr_instr.z < -1.742) {
                curr_instr.x *= -1.0;
                curr_instr.y *= -1.0;
                curr_instr.z += M_PI;
            }



            for (int j = 0; j < NUM_ROATATIONS; ++j) {
                direction = curr_instr.z < 0 ? -1 : 1;

                // Create copies since these get modified in the equations and used after, also looks clean
                x_cpy = curr_instr.x_coord;
                y_cpy = curr_instr.y_coord;

                // std::cout << "x_cpy >> j  " << (x_cpy >> j).to_float() << std::endl;
                // std::cout << "y_cpy >> j  " << (y_cpy >> j).to_float() << std::endl;


                //select which direction
                // shift bits to rotate in correct direction
                if (curr_instr.z < 0) {
                    // instead of implementing subtraction, im gonna flip the sign then add y_cpy
                    // implementation of add will show this is equivalent
                    curr_instr.x_coord = x_cpy + (y_cpy >> j);
                    curr_instr.y_coord = y_cpy + (x_cpy >> j).flip_sign();
                } else {
                    curr_instr.x_coord = x_cpy + (y_cpy >> j).flip_sign();
                    curr_instr.y_coord = y_cpy + (x_cpy >> j);
                }
                
                std::cout << curr_instr.x_coord.to_float() << "  " << curr_instr.y_coord.to_float() << "  " << curr_instr.z << std::endl;
                
                // use angle from look up table 
                curr_instr.z = curr_instr.z - direction * angles[j];
            }
        } 
        // Vector Mode
        else {
            // Adjust for overflow if desired rotation is larger than is possible with given angles
            // If the vector is not in the first quadrant, we need to adjust it and add a starting value to z
            // If in the third quadrant, adjust start point to first quadrant and add 180 degrees to z
            if (curr_instr.x < 0 && curr_instr.y < 0) {
                curr_instr.x *= -1.0;
                curr_instr.y *= -1.0;
                curr_instr.z += M_PI;
            } 
            // If in the second quadrant, flip x coord and add 90 degrees to z
            else if (curr_instr.x < 0) {
                curr_instr.x *= -1.0;
                curr_instr.z += M_PI;
            } 
            // If in the fourth quadrant, flip y coord and add 270 degrees to z
            else if (curr_instr.y < 0) {
                curr_instr.y *= -1.0;
                curr_instr.z += 3.0 * M_PI / 2.0;
            }   

            
            for (int j = 0; j < NUM_ROATATIONS; ++j) {
                // Pick direction
                if (curr_instr.x_coord.magnitude == 0 && curr_instr.y_coord.magnitude == 0){
                    direction = -1;

                } else if (curr_instr.y_coord.magnitude == 0) {
                    direction = (curr_instr.x_coord.sign) == 1 ? 1 : -1;

                } else if (curr_instr.x_coord.magnitude == 0) {
                    direction = (curr_instr.y_coord.sign == 1) ? 1 : -1;

                } else {
                    int temp = curr_instr.x_coord.sign + curr_instr.y_coord.sign;
                    direction = (temp == 1)  ? 1 : -1;
                    
                }


                // Create copies since these get modified in the equations and used after, also looks clean
                x_cpy = curr_instr.x_coord;
                y_cpy = curr_instr.y_coord;

                // // shift bits to rotate in correct direction
                // curr_instr.x_temp = x_cpy - direction * (y_cpy >> j);
                // curr_instr.y_temp = y_cpy + direction * (x_cpy >> j);
                

                 // shift bits to rotate in correct direction
                if (direction < 0) {
                    // instead of implementing subtraction, im gonna flip the sign then add y_cpy
                    // implementation of add will show this is equivalent
                    curr_instr.x_coord = x_cpy + (y_cpy.flip_sign() >> j).flip_sign();
                    curr_instr.y_coord = y_cpy + (x_cpy.flip_sign() >> j);
                } else {
                    curr_instr.x_coord = x_cpy + (y_cpy >> j).flip_sign();
                    curr_instr.y_coord = y_cpy + (x_cpy >> j);
                }
                
                
                // use angle from look up table 
                curr_instr.z = curr_instr.z - direction * angles[j];
            }

        
        }


        // Use sum/difference of different shifts to approximate multiplication by 0.607352
        curr_instr.x_coord = (curr_instr.x_coord >> 1) + (curr_instr.x_coord >> 3) + (curr_instr.x_coord >> 6) + (curr_instr.x_coord >> 5).flip_sign();
        curr_instr.y_coord = (curr_instr.y_coord >> 1) + (curr_instr.y_coord >> 3) + (curr_instr.y_coord >> 6) + (curr_instr.y_coord >> 5).flip_sign();
        

        // Convert back to float type
        curr_instr.x = curr_instr.x_coord.to_float();
        curr_instr.y = curr_instr.y_coord.to_float();

        // write answers to file
        output_file << "x=" << curr_instr.x << ", y=" << curr_instr.y << ", z=" << curr_instr.z << "\n";

    }


    output_file.close();
    return 0;
}
