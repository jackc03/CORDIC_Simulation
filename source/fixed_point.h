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

struct fixed_30;

#define NUM_SIGN 1
#define NUM_INTS 4
#define NUM_FRACT_18 13
#define NUM_FRACT_30 25



// Data type to read 16 bit input into, and to return as our 18 bit output
// In hardware this would use 2's complement, but for ease of programming i am doing signed magnitude in the simulator
typedef struct fixed_18 {
    unsigned int sign      : 1;
    unsigned int magnitude : NUM_SIGN + NUM_INTS + NUM_FRACT_18;
    
    fixed_18 operator=(const float& x){
        sign = (x < 0) ? 1 : 0;

        float whole_part_f, fract_f;
        int whole_part_i, fract_i;

        fract_f = std::modf(x, &whole_part_f);

        //my way of doing abs(int_part)
        whole_part_i = (int)(whole_part_f * (whole_part_f / whole_part_f));

        fract_i = (int)(fract_f * (2 << 13));

        magnitude = (whole_part_i << 13) | fract_i;

        return *this;
    }

    float to_float(){
        float return_var;
        float pos_or_neg = (sign == 1) ? -1 : 1;
        float mag = ((float)(magnitude >> 13) + ((float)(magnitude & 0b1111111111111))/(2,13));
        return_var = pos_or_neg * mag;
        return return_var;
    }

    fixed_18 operator=(const fixed_30& old_fixed);


    void print(){
        std::cout << "Value = " << to_float() << std::endl;
    }

} fixed_18;


// Data type with extra padding on fraction bit to prevent data loss during intermediate calculations
// In hardware this would use 2's complement, but for ease of programming i am doing signed magnitude in the simulator
typedef struct fixed_30 {
    unsigned int sign      : 1;
    unsigned int magnitude : NUM_INTS + NUM_FRACT_30;


    fixed_30 operator=(const fixed_18& old_fixed) {
        int whole_part_i, fract_i;

        sign = old_fixed.sign;


        whole_part_i = (old_fixed.magnitude >> 13);

        // Convert old magnitude to float, so then i can scale up to new bit width, then convert to int
        fract_i = (int) ((float)(old_fixed.magnitude & 0b1111111111111)/(2 << 13) * (2 << 25));

        magnitude = (whole_part_i << 25) | fract_i;

        return *this;

        
    }


    fixed_30 operator>>(const int& x) const {
        struct fixed_30 result;
        result.sign = this->sign;
        result.magnitude = (this->magnitude) >> x;
        return result;
    }

    fixed_30 operator=(const float& x){
        this->sign = (x < 0) ? 1 : 0;

        float whole_part_f, fract_f;
        unsigned int whole_part_i, fract_i;

        fract_f = std::modf(x, &whole_part_f);

        //my way of doing abs(int_part)
        whole_part_i = (int) ((whole_part_f) * ((this->sign == 1) ? -1 : 1));


        fract_i = (int)(fract_f * ((this->sign == 1) ? -1 : 1) * (2 << 25));

        this->magnitude = (whole_part_i << 25) | fract_i;

        return *this;
    }

    void operator=(const fixed_30& x) {
        this->sign = x.sign;
        this->magnitude = x.magnitude;

    }



    float to_float(){
        float return_var;
        float pos_or_neg = (sign == 1) ? -1 : 1;
        float mag = ((float)(magnitude >> 25)) + ((float)(magnitude & 0b1111111111111111111111111))/(2 << 25);
        return_var = pos_or_neg * mag;
        return return_var;
    }

    fixed_30 flip_sign() {
        fixed_30 return_var;

        return_var.sign = ~(this->sign);
        return_var.magnitude = this->magnitude;
        return return_var;
    }



    void print(){
        std::cout << "Value = " << to_float() << std::endl;
    }

    // this operator definitely misses edge cases, but luckily, i am guaranteed to only add or subtract by a smaller number so I think I skirt around them
    fixed_30 operator+(const fixed_30& x) {
        // Whole and fraction of the current fixed_30(self)
        int s_whole, s_fract;

        // Whole and fraction for x
        int x_whole, x_fract;

        int accumulation;

        fixed_30 return_fix;

        //get whole and fractional part for each num
        s_whole = this->magnitude >> 25;
        x_whole = x.magnitude >> 25;
        s_fract = this->magnitude & 0b1111111111111111111111111;
        x_fract = x.magnitude & 0b1111111111111111111111111;
        // if number is posistive
        if ((this->sign == 0 && x.sign == 0) || (this->sign == 1 && x.sign == 1)) {
            return_fix.sign = this->sign;

            accumulation = (s_fract + x_fract) / (2 << 25);

            
            
            return_fix.magnitude = ((s_whole + x_whole + accumulation) << 25) | (s_fract + x_fract);

        } else if (this->sign == 1 && x.sign == 0) {

            
            // ugh adding two numbers is hardddddd
            if (s_whole > x_whole) {
                //neg number bigger so stay neg
                return_fix.sign = 1;
                if (s_fract > x_fract) {
                    return_fix.magnitude = ((s_whole - x_whole) << 25) | (s_fract - x_fract);
                } else {
                    accumulation = 1;
                    // take one from whole number and add to fract, ughhhh am i an elementary school child; jkjk
                    return_fix.magnitude = ((s_whole - (x_whole + 1)) << 25) | ( (s_fract + (int)(2 << 25)) - x_fract);
                }
            } else if (s_whole < x_whole) {
                // neg number smaller so become positive
                return_fix.sign = 0;

                if (s_fract < x_fract) {
                    return_fix.magnitude = ((x_whole - s_whole) << 25) | (x_fract - s_fract);
                } else {
                    accumulation = 1;
                    // take one from whole number and add to fract, ughhhh am i an elementary school child; jkjk
                    return_fix.magnitude = ((x_whole - (s_whole + 1)) << 25) | ( (x_fract + (int)(2 << 25)) - s_fract);
                }
            } else {
                if (s_fract > x_fract) {
                    return_fix.sign = 1;
                    return_fix.magnitude =  (s_fract - x_fract);
                } else {
                    return_fix.sign = 0;

                    return_fix.magnitude =  x_fract - s_fract;
                }
            }


        }  else {
            // else if (this->sign == 0 && x.sign == 1)  

            // ugh adding two numbers is hardddddd
            if (s_whole > x_whole) {

                //pos number bigger so stay pos
                return_fix.sign = 0;

                if (s_fract > x_fract) {
                    return_fix.magnitude = ((s_whole - x_whole) << 25) | (s_fract - x_fract);
                } else {
                    accumulation = 1;
                    // take one from whole number and add to fract, ughhhh am i an elementary school child; jkjk
                    return_fix.magnitude = ((s_whole - (x_whole + 1)) << 25) | ( (s_fract + (int)(2 << 25)) - x_fract);
                }

            } else if (s_whole < x_whole) {

                // pos number smaller so become neg
                return_fix.sign = 1;

                if (x_fract > s_fract) {
                    return_fix.magnitude = ((x_whole - s_whole) << 25) | (x_fract - s_fract);
                } else {
                    accumulation = 1;
                    // take one from whole number and add to fract, ughhhh am i an elementary school child; jkjk
                    return_fix.magnitude = ((x_whole - (s_whole + 1)) << 25) | ( (x_fract + (int)(2 << 25)) - s_fract);
                }
            } else {
                if (s_fract > x_fract) {
                    return_fix.sign = 0;
                    return_fix.magnitude =  (s_fract - x_fract);
                } else {
                    return_fix.sign = 1;

                    return_fix.magnitude =  x_fract - s_fract;
                }
            }

        }

        return return_fix;
    }


    // fixed_30 operator-(const fixed_30& x) {
    //     // Whole and fraction of the current fixed_30(self)
    //     int s_whole, s_fract;

    //     // Whole and fraction for x
    //     int x_whole, x_fract;

    //     fixed_30 return_fix;

    //     // if number is posistive
    //     if ((this->sign == 0 && x.sign == 0) || (this->sign == 1 && x.sign == 1)) {
    //         return_fix.sign = this->sign;
    //         s_whole = this->magnitude >> 25;
    //         x_whole = x.magnitude >> 25;
    //         s_fract = this->magnitude & 0b1111111111111;
    //         x_fract = x.magnitude & 0b1111111111111;
            
    //         return_fix.magnitude = ((s_whole + x_whole) << 25) | (s_fract + x_fract);

    //     } else {
    //         //Separate out whole and fraction parts, then subtract and combine
    //         s_whole = this->magnitude >> 25;
    //         x_whole = x.magnitude >> 25;
    //         s_fract = this->magnitude & 0b1111111111111;
    //         x_fract = x.magnitude & 0b1111111111111;
            
    //         return_fix.magnitude = ((s_whole - x_whole) << 25) | (s_fract - x_fract);

    //     } 
    // }
    


} fixed_30;




// down at bottom so i can use full definition of fixed_30
fixed_18 fixed_18::operator=(const fixed_30& old_fixed) {
    int whole_part_i, fract_i;

    sign = old_fixed.sign;


    whole_part_i = (old_fixed.magnitude >> 25);

    // Convert old magnitude to float, so then i can scale up to new bit width, then convert to int
    fract_i = (int) ((float)(old_fixed.magnitude & 0b1111111111111111111111111)/(2 << 25) * (2 << 13));

    magnitude = (whole_part_i << 13) | fract_i;

    return *this;

    
}

