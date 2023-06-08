#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <random>
#include <queue>
#include "m1.h"
#include "m2.h"
#include "m3.h"
#include "m4.h"
#include "dataStructures.h"
#include <chrono>
#include <bits/stdc++.h>
using namespace std::chrono;



//data structures
std::vector<IntersectionIdx> delivery_depot_ids;
std::vector<std::vector <double> > travelTimeMatrix;
std::vector<std::vector <std::vector<StreetSegmentIdx>>> subPath;
std::vector<CourierSubPath> solution_path;
int repeated_intersections;
std::unordered_map<IntersectionIdx, std::vector<int>> repeated_nodes;

bool overall_timer(auto overall_start);
bool check_element_exist(std::vector<int> route, int element);
std::vector<std::vector<double>> computeMatrix(std::vector<IntersectionIdx> delivery_depot_ids,const float turn_penalty);
void SSMD_Dijkstra(int row, IntersectionIdx start_intersection, const float turn_penalty, int delivery_matrix_size, std::vector<IntersectionIdx> important_ids, std::unordered_map<IntersectionIdx, std::vector<int>> repeated_ids);
void computeMatrix(const float turn_penalty, int matrix_size, int delivery_matrix_size);
void getDeliveriesIntersectionIds(const std::vector<DeliveryInf>& deliveries);
std::vector<int> introduce_Randomness (std::vector<int> greedy_result);
int if_intersection_found(IntersectionIdx id, int total_destinations, std::vector<IntersectionIdx> important_ids);
bool check_key(IntersectionIdx key);
void reduce_temp(int & change_temp);

////Specifies a delivery order (input to your algorithm).
////To satisfy the order the item-to-be-delivered must have been picked-up 
////from the pickUp intersection before visiting the dropOff intersection.
//struct DeliveryInf {
//
//    // Constructor
//    DeliveryInf(IntersectionIdx pick_up, IntersectionIdx drop_off): pickUp(pick_up), dropOff(drop_off) {}
//
//    //The intersection id where the item-to-be-delivered is picked-up.
//    IntersectionIdx pickUp;
//
//    //The intersection id where the item-to-be-delivered is dropped-off.
//    IntersectionIdx dropOff;
//};
//
//
//// Specifies one subpath of the courier truck route
//struct CourierSubPath {
//
//    // The intersection id where a start depot, pick-up intersection or 
//    // drop-off intersection is located
//    IntersectionIdx start_intersection;
//
//    // The intersection id where this subpath ends. This must be the 
//    // start_intersection of the next subpath or the intersection of an end depot
//    IntersectionIdx end_intersection;
//
//    // Street segment ids of the path between start_intersection and end_intersection 
//    // They form a connected path (see m3.h)
//    std::vector<StreetSegmentIdx> subpath;
//};

bool check_key(IntersectionIdx key){
    // Key is not present
    if (repeated_nodes.find(key) == repeated_nodes.end())
        return false;
 
    return true;
}


std::vector<CourierSubPath> travelingCourier(const std::vector<DeliveryInf>& deliveries, 
                                             const std::vector<IntersectionIdx>& depots, const float turn_penalty){
    // std::cout<<std::endl<<std::endl;
    //////////////////OVERALL TIMER//////////////////////
    static auto overall_start = std::chrono::steady_clock::now(); // #1

    //std::cout<<std::endl;
    getDeliveriesIntersectionIds(deliveries); //initialize delivery_depot_ids
    int deliveries_num = deliveries.size();
    //std::cout<<"num of deliveries:"<<deliveries_num<<std::endl;
    //std::cout<<"num of depots:"<<depots.size()<<std::endl;
    int delivery_matrix_size = 2 * deliveries_num;
    //print out delivery ids
    // for (auto elem :delivery_depot_ids ) {
    //     std::cout << elem << " ";
    // }
    // std::cout<<std::endl;
    
    //add depots into the vector
    for(int i = 0; i < depots.size(); i++){
        delivery_depot_ids.push_back(depots[i]);  
    }
    
    //std::cout<<"full vector:"<<std::endl;
//    for (auto elem :delivery_depot_ids ) {
//        std::cout << elem << " ";
//    }
    
    int full_matrix_size = delivery_depot_ids.size();

    //Computing ssmd & matrix time
    auto start = high_resolution_clock::now();
    
    computeMatrix(turn_penalty, full_matrix_size, delivery_matrix_size);
    
    
    
//    
//    for (auto x : repeated_nodes){
//        std::cout<<"the repeating intersect "<<x.first<<std::endl;
//        for(auto y: x.second)
//            std::cout<<y<<" ";
//        std::cout<<std::endl;
//    }
    

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);
 
    // To get the value of duration use the count()
    // member function on the duration object
    std::cout << "SSMD and matrix building time: " << (double) (duration.count() / 1000000.0) << std::endl;


 
//    for (int i = 0; i < travelTimeMatrix.size(); i++)
//	{
//		for (int j = 0; j < travelTimeMatrix[i].size(); j++) {
//			std::cout << travelTimeMatrix[i][j] << "   ";
//		}	
//		std::cout << std::endl;
//	}
    
    //Computing Greedy running time
    auto start2 = high_resolution_clock::now();


    std::vector<int> route;
    std::vector<int> greedy_result;
    std::vector<int> greedy_copy;
    std::vector<int> greedy_static;
    int row;
    int num_row_traversed;
    int next_row;
    int change_temp = 4500000;
    double total_time;
    double path_shortest_time = INFINITY;
    
    //start from every pick-up point, compare and choose the smallest route.
    for (int iteration = 0; iteration < deliveries.size(); iteration++) {
        //the starting point should be a pick-up location
        row = iteration*2;
        
        num_row_traversed = 0;
        route.clear();
        route.push_back(row);
        total_time = 0;
        
	    //Greedy algorithm for each starting pick-up
        while (num_row_traversed < delivery_matrix_size ) {
            double time = INFINITY;

            for (int col = 0; col < delivery_matrix_size; col++) {
                if (col == row && col != delivery_matrix_size - 1) {
                    continue;
                } 

                if (travelTimeMatrix[row][col] < time) {
                    if (!check_element_exist(route, col)) {
                        if ((col % 2) == 0) {  ////col is even: pick-up
                            //update
                            time = travelTimeMatrix[row][col];
                            next_row = col;
                        }
                        //// col is odd => drop-off
                        else {
                           
//                            //check if the pick-up counterpart has repeats
//                            if(check_key(delivery_depot_ids[col-1])){
//                                 //std::cout<<"hi "<<col-1<<" is repeating"<<std::endl;
//                                
//                                //repeated intersection
//                                std::vector<int> repeated_vec = repeated_nodes[delivery_depot_ids[col-1]];
//                                bool update = false;
//                                
//                                for (int val : repeated_vec) {
//                                    if(check_element_exist(route, val)){
//                                        //update
//                                        time = travelTimeMatrix[row][col];
//                                        next_row = col;
//                                        update = true;
//                                        break;
//                                    }
//                                        
//                                }
//                                if(!update){
//                                    for(int i = 0; i < repeated_vec[0]; i ++){
//                                        if(delivery_depot_ids[i] == delivery_depot_ids[col-1] && check_element_exist(route, i)){
//                                            //update
//                                            time = travelTimeMatrix[row][col];
//                                            next_row = col;    
//                                        }
//                                    }
//                                }
//                                if(update){
////                                    std::cout<<"current col is:"<<col<<std::endl;
//                                    std::cout<<"current route is "<<std::endl;                          
//                                  //print out the current route
//                                  for(int i = 0; i<route.size(); i++){
//                                      std::cout<<route[i]<<" ";
//                                  }
//                                  std::cout<<std::endl;
//
//   
//                                  
//                                  std::cout<<update<<std::endl;
//                                }
//                                  
//                                
//                                    
//                            }
//                            //pick-up counterpart has no repeats
//                            else{
                                //check if the pick-up counterpart is already traversed
                                if (check_element_exist(route, col - 1)) {
                                    //update
                                    time = travelTimeMatrix[row][col];
                                    next_row = col;
                                }
                           // }
                        }
                    }   
                }
                
                //jump to next row
                if (col == delivery_matrix_size-1) {
                    num_row_traversed++;
                    //cout<<"["<<row<<"]"<<"["<<next_row<<"]:"<<travelTimeMatrix[row][next_row]<<endl;
                    if (row != next_row) {
                        total_time += travelTimeMatrix[row][next_row];
                    }
                    if (num_row_traversed < delivery_matrix_size) {
                        row = next_row;
                        route.push_back(next_row);
                    }     
                }
            }        
        }
      
        
        greedy_copy.resize(route.size());
        greedy_static.resize(route.size());

        if (total_time < path_shortest_time) {
            greedy_result = route;
            greedy_copy = route;
            greedy_static = route;
            path_shortest_time = total_time;
        }            
    }
    

    //std::cout<<"greedy done"<<std::endl;
    
    //print out the greedy result
    //    for (int i = 0; i<route.size(); i++) {
    //        std::cout<<greedy_result[i]<<" ";
    //    }
    //    std::cout<<std::endl;
    
    //finding the starting depot
    double depot_to_first = INFINITY;
    int first_destination_idx = greedy_result[0];
    int depot_index;

    //check vertically
    for (int depot_row = delivery_matrix_size; depot_row < full_matrix_size; depot_row++) {
        //std::cout<<"depot_row is:"<<depot_row<<" time is: "<<travelTimeMatrix[depot_row][first_destination_idx]<<std::endl;
        if (travelTimeMatrix[depot_row][first_destination_idx] < depot_to_first) {
            depot_index = depot_row;
            depot_to_first = travelTimeMatrix[depot_row][first_destination_idx];
        }      
    }

    //std::cout<<"the depot to start is:"<<depot_index<<std::endl;
    greedy_result.insert(greedy_result.begin(), depot_index);
    //she d::cout<<"here"<<std::endl;
    
    //finding the depot to return
    double last_to_depot = INFINITY;
    int last_destination_idx = greedy_result[delivery_matrix_size-1];

    //check horizontally
    for (int col = delivery_matrix_size; col < full_matrix_size; col++) {
        if (travelTimeMatrix[last_destination_idx][col] < last_to_depot) {
            depot_index = col;
            last_to_depot = travelTimeMatrix[last_destination_idx][col];
        }      
    }

    //std::cout<<"the depot to return is:"<<depot_index<<std::endl;
    greedy_result.push_back(depot_index);
    //std::cout<<"the size of greedy result is:"<<greedy_result.size()<<std::endl;
    
    //print out the greedy result
    //    for (int i = 0; i<greedy_result.size(); i++) {
    //        std::cout<<greedy_result[i]<<" ";
    //    }
    //std::cout<<std::endl;
   
    solution_path.resize(greedy_result.size()-1);
    //std::cout<<" ";
    
     for (int i = 0; i < greedy_result.size()-1; i++) {
            //std::cout<<"i is:"<<i<<std::endl;
            solution_path[i].start_intersection = delivery_depot_ids[greedy_result[i]];
            //std::cout<<solution_path[i].start_intersection<<" ";
            
            solution_path[i].end_intersection = delivery_depot_ids[greedy_result[i+1]];
            //std::cout<<solution_path[i].end_intersection<<" \n";
            solution_path[i].subpath = subPath[greedy_result[i]][greedy_result[i+1]];   
        }

    
    
    auto stop2 = high_resolution_clock::now();

    auto duration2 = duration_cast<microseconds>(stop2 - start2);
 
    // To get the value of duration use the count()
    // member function on the duration object
    // std::cout << "Greedy running time: " << (double) (duration2.count() / 1000000.0) << std::endl;

        //int counter = 100;
    int times_result_got_opted = 0;

    double prev_time = 0;

    //Randomly edit the path to hopefully find a better result 

    // std::cout << "greedy result: " << std::endl;
    // for (int element : greedy_copy)
    // {
    //     std::cout << element << " ";
    // }
    // std::cout << std::endl;
    


    int count_of_the_while_loop_per_48_seconds = 0; //2154245*8
    
    while (1) {
        count_of_the_while_loop_per_48_seconds++;
        if (overall_timer(overall_start)) 
        //if(counter == 0)
        {
            // std::cout << "returned path" << std::endl;
            // std::cout << "times that the result got optimized: " << times_result_got_opted << std::endl;

            std::cout << "the loop loops " << count_of_the_while_loop_per_48_seconds << " times in 48 s" << std::endl;
            return solution_path;
        }

        bool legal = true;

        
        
        std::vector<int> new_route = introduce_Randomness(greedy_copy); 

        for (int element : new_route) {
            //this is a drop off point
            if ((element % 2) == 1) {  
                //check if the pick-up counterpart is already traversed
                //////not sure if this works
                //std::cout << "checking legality, pick up before drop off" << std::endl;
                auto drop_off = find(new_route.begin(), new_route.end(), element);
                auto pick_up  = find(new_route.begin(), new_route.end(), element - 1);
                //std::cout << "checking legality, pick up before drop off END" << std::endl;
	            // If the pick-up point doesn't exist prior than the drop off
	            if (pick_up > drop_off) {
                    legal = false;
                    break;
                }
            }
        }

        if (legal) {
            double new_route_time = 0; 

            // std::cout << "entered find new legal" << std::endl;

            for (int element_pt = 0; element_pt < new_route.size() - 1;  element_pt++) {
                //std::cout << "cp1" << std::endl;
                new_route_time = new_route_time + travelTimeMatrix[new_route[element_pt]][new_route[element_pt + 1]];
            }

            double delta_C = new_route_time - path_shortest_time; //- prev_time;

            auto now = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            std::srand(ms);

            if (delta_C < 0.0 /* ||((double)rand())/RAND_MAX < exp(-1*(delta_C/change_temp))*/) {
               greedy_copy = new_route;
               prev_time = new_route_time;
            }
            
            if (new_route_time < path_shortest_time) {
                greedy_copy = new_route;
                prev_time = new_route_time;
                greedy_result = new_route;
                greedy_static = new_route;
                path_shortest_time = new_route_time;

                //  std::cout << "better than greedy found: " << std::endl;
                //  for (int element : greedy_result)
                //  {
                //      std::cout << element << " ";
                //  }

                //finding the starting depot
                double depot_to_first = INFINITY;
                int first_destination_idx = greedy_result[0];
                int depot_index;

                //check vertically
                for (int depot_row = delivery_matrix_size; depot_row < full_matrix_size; depot_row++) {
                    //std::cout<<"depot_row is:"<<depot_row<<" time is: "<<travelTimeMatrix[depot_row][first_destination_idx]<<std::endl;
                    if (travelTimeMatrix[depot_row][first_destination_idx] < depot_to_first) {
                        depot_index = depot_row;
                        depot_to_first = travelTimeMatrix[depot_row][first_destination_idx];
                    }      
                }
                //std::cout<<"check pt 1" << std::endl;
                //std::cout<<"the depot to start is:"<<depot_index<<std::endl;
                greedy_result.insert(greedy_result.begin(), depot_index);
                //she d::cout<<"here"<<std::endl;
                
                //finding the depot to return
                double last_to_depot = INFINITY;
                int last_destination_idx = greedy_result[delivery_matrix_size-1];

                //check horizontally
                for (int col = delivery_matrix_size; col < full_matrix_size; col++) {
                    if (travelTimeMatrix[last_destination_idx][col] < last_to_depot) {
                        depot_index = col;
                        last_to_depot = travelTimeMatrix[last_destination_idx][col];
                    }      
                }
                //std::cout<<"check pt 2" << std::endl;

                //std::cout<<"the depot to return is:"<<depot_index<<std::endl;
                greedy_result.push_back(depot_index);
                solution_path.clear();
                
                solution_path.resize(greedy_result.size()-1);
                //std::cout<<"check pt 3" << std::endl;
                
                for (int i = 0; i < greedy_result.size()-1; i++) {
                    //std::cout<<"i is:"<<i<<std::endl;
                    solution_path[i].start_intersection = delivery_depot_ids[greedy_result[i]];
                    //std::cout<<solution_path[i].start_intersection<<" ";
                        
                    solution_path[i].end_intersection = delivery_depot_ids[greedy_result[i+1]];
                    //std::cout<<solution_path[i].end_intersection<<" \n";
                    solution_path[i].subpath = subPath[greedy_result[i]][greedy_result[i+1]];   
                }
                //std::cout<<"check pt 4" << std::endl;
            }

            //std::cout << change_temp << " " << change_temp << " " << change_temp << " " << change_temp << " " << change_temp << std::endl;
        } else {
            greedy_copy = greedy_static;
            prev_time = path_shortest_time;
        }

        new_route.clear();
        reduce_temp(change_temp);
         
        //counter = counter - 1;
        //std::cout << counter << std::endl;
        
    }
    
}

void reduce_temp(int & change_temp) {
    change_temp--;

    if (change_temp <= 0) {
        change_temp = 1;
    }
}

bool overall_timer(auto overall_start)
{
    auto now = std::chrono::steady_clock::now(); 
    if (now - overall_start >= std::chrono::milliseconds(49500))
    {
        return true;
    }
    return false;
}




/// 7 Ways of Randomness Optimization:
std::vector<int> introduce_Randomness (std::vector<int> greedy_result)
{
    //std::cout << "entered randomness" << std::endl;
    //auto now = std::chrono::system_clock::now().time_since_epoch();
    //auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
    //std::srand(ms);

    std::random_device rd1;

    // Use the seed to initialize a random number engine
    std::default_random_engine rng(rd1());

    // Create a custom discrete probability distribution
    std::vector<double> probabilities = {0.02, 0.02, 0.48, 0.48}; // 4, 5, 6, 7
    std::discrete_distribution<int> dist4(probabilities.begin(), probabilities.end());

    // Generate a random integer based on the custom probability distribution
    int biased_optcase = dist4(rng) + 4; // Add 4 to get the numbers in the desired range (4, 5, 6, 7)



    //Increase the possibility of getting a random number between 4 to 7 higher than getting a random number between 1 to 3
    //if (random_optcase <= 6) 
    //{
    //    biased_optcase = random_optcase % 4 + 4;
    //} else 
    //{
     //   biased_optcase = random_optcase % 3 + 1;
    //}

    

    switch (biased_optcase) 
    {
        case 1:
        {
            //std::cout << "rand 1 implemented" << std::endl;
            /*====================================================================================
             1. find two random data in the vector and reverse ordering every element in between
            ======================================================================================*/
            auto now = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            std::srand(ms);

            int path_size = greedy_result.size();
            int rand_pt_1 = rand() % path_size;
            int rand_pt_2 = -1;
            while (rand_pt_2 == -1 || rand_pt_2 == rand_pt_1)
            {
                rand_pt_2 = rand() % path_size;
            }
            if(rand_pt_2 < rand_pt_1) //point 2 value > point 1
            {
                int temp = rand_pt_1;
                rand_pt_1 = rand_pt_2;
                rand_pt_2 = temp;
            }
                //point 1: random starting point
                //point 2: random ending point 


            std::reverse(greedy_result.begin() + rand_pt_1, greedy_result.begin() + rand_pt_2);
            return greedy_result;
        }
        case 2:
        {
            //std::cout << "rand 2 implemented" << std::endl;
            /*====================================================================================
                    2. find two random data swap the two elements
            ======================================================================================*/
            auto now = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            std::srand(ms);

            int path_size = greedy_result.size();
            int rand_pt_1 = rand() % path_size;
            int rand_pt_2 = -1;
            while (rand_pt_2 == -1 || rand_pt_2 == rand_pt_1)
            {
                rand_pt_2 = rand() % path_size;
            }
            if(rand_pt_2 < rand_pt_1) //point 2 value > point 1
            {
                int temp = rand_pt_1;
                rand_pt_1 = rand_pt_2;
                rand_pt_2 = temp;
            }
                //point 1: random starting point
                //point 2: random ending point 


            std::swap(greedy_result[rand_pt_1], greedy_result[rand_pt_2]);
            return greedy_result;
        }
        case 3:
        {
            //std::cout << "rand 3 implemented" << std::endl;
            /*====================================================================================
                3. find two random data in the vector and randomly shuffle the elements in between
            ======================================================================================*/
            auto now = std::chrono::system_clock::now().time_since_epoch();
            auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            std::srand(ms);

            int path_size = greedy_result.size();
            int rand_pt_1 = rand() % path_size;
            int rand_pt_2 = -1;
            while (rand_pt_2 == -1 || rand_pt_2 == rand_pt_1)
            {
                rand_pt_2 = rand() % path_size;
            }
            if(rand_pt_2 < rand_pt_1) //point 2 value > point 1
            {
                int temp = rand_pt_1;
                rand_pt_1 = rand_pt_2;
                rand_pt_2 = temp;
            }
                //point 1: random starting point
                //point 2: random ending point 

            std::default_random_engine rng(static_cast<unsigned int>(ms));

            std::shuffle(greedy_result.begin() + rand_pt_1, greedy_result.begin() + rand_pt_2 + 1, rng);
            return greedy_result;
        }
        case 4: 
        {
            //std::cout << "rand 4 implemented" << std::endl;
            /*====================================================================================
                    4. 2.5 opt (1) right shift one element
            ======================================================================================*/ 
            //auto now = std::chrono::system_clock::now().time_since_epoch();
            //auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            //std::srand(ms);

            int path_size = greedy_result.size();

            int min_value = 0;
            int max_value = path_size - 1;
            // Use std::random_device to generate a seed
            std::random_device rd;
            // Use the seed to initialize a random number engine
            std::default_random_engine rng(rd());
            // Create a uniform integer distribution within the desired range
            std::uniform_int_distribution<int> dist(min_value, max_value);

            
            //int rand_pt_1 = rand() % path_size;
            int rand_pt_1 = dist(rng);
            int rand_pt_2 = -1;
            while (rand_pt_2 == -1 || rand_pt_2 == rand_pt_1)
            {
                rand_pt_2 = dist(rng);
                //rand_pt_2 = rand() % path_size;
            }
            if(rand_pt_2 < rand_pt_1) //point 2 value > point 1
            {
                int temp = rand_pt_1;
                rand_pt_1 = rand_pt_2;
                rand_pt_2 = temp;
            }
                //point 1: random starting point
                //point 2: random ending point 


            std::rotate(greedy_result.begin() + rand_pt_1, greedy_result.begin() + rand_pt_2, greedy_result.begin() + rand_pt_2 + 1);
            return greedy_result;
        }
        case 5:
        {
            //std::cout << "rand 5 implemented" << std::endl;
            /*====================================================================================
                        5. 2.5 opt (2) left shift one element
            ======================================================================================*/
            //auto now = std::chrono::system_clock::now().time_since_epoch();
            //auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            //std::srand(ms);

            int path_size = greedy_result.size();

            int min_value = 0;
            int max_value = path_size - 1;
            // Use std::random_device to generate a seed
            std::random_device rd;
            // Use the seed to initialize a random number engine
            std::default_random_engine rng(rd());
            // Create a uniform integer distribution within the desired range
            std::uniform_int_distribution<int> dist(min_value, max_value);

            
            //int rand_pt_1 = rand() % path_size;
            int rand_pt_1 = dist(rng);
            int rand_pt_2 = -1;
            while (rand_pt_2 == -1 || rand_pt_2 == rand_pt_1)
            {
                rand_pt_2 = dist(rng);
                //rand_pt_2 = rand() % path_size;
            }
            if(rand_pt_2 < rand_pt_1) //point 2 value > point 1
            {
                int temp = rand_pt_1;
                rand_pt_1 = rand_pt_2;
                rand_pt_2 = temp;
            }
                //point 1: random starting point
                //point 2: random ending point 

            std::rotate(greedy_result.begin() + rand_pt_1, greedy_result.begin() + rand_pt_1 + 1, greedy_result.begin() + rand_pt_2 + 1);
            return greedy_result;
        }
        case 6:
        {
            //std::cout << "rand 6 implemented" << std::endl;
            /*====================================================================================
                6. or opt (1) right shift by random number of elements
            ======================================================================================*/
            //auto now = std::chrono::system_clock::now().time_since_epoch();
            //auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            //std::srand(ms);

            int path_size = greedy_result.size();

            int min_value = 0;
            int max_value = path_size - 1;
            // Use std::random_device to generate a seed
            std::random_device rd;
            // Use the seed to initialize a random number engine
            std::default_random_engine rng(rd());
            // Create a uniform integer distribution within the desired range
            std::uniform_int_distribution<int> dist(min_value, max_value);

            int rand_pt_1 = dist(rng);
            //int rand_pt_1 = rand() % path_size;
            
            std::uniform_int_distribution<int> dist2(0, path_size - rand_pt_1 - 1);

            int rand_pt_2 = rand_pt_1 + dist2(rng);
            //int rand_pt_2 = rand_pt_1 + (std::rand() % (path_size - rand_pt_1));

            std::uniform_int_distribution<int> dist3(0, rand_pt_2 - rand_pt_1);
            int shift = dist3(rng);
            //int shift = std::rand() % (rand_pt_2 - rand_pt_1 + 1);

            //std::cout << "Start: " << rand_pt_1 << ", End: " << rand_pt_2 << ", Shift: " << shift << std::endl;

            int section_size = rand_pt_2 - rand_pt_1 + 1;
            std::rotate(greedy_result.begin() + rand_pt_1, greedy_result.begin() + rand_pt_2 - shift + 1, greedy_result.begin() + rand_pt_2 + 1);

            return greedy_result;
        }
        case 7:
        {
            //std::cout << "rand 7 implemented" << std::endl;
            /*====================================================================================
                7. or opt (2) left shift by random number of elements
            ======================================================================================*/
            //auto now = std::chrono::system_clock::now().time_since_epoch();
            //auto ms = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();
            //std::srand(ms);

            int path_size = greedy_result.size();

            int min_value = 0;
            int max_value = path_size - 1;
            // Use std::random_device to generate a seed
            std::random_device rd;
            // Use the seed to initialize a random number engine
            std::default_random_engine rng(rd());
            // Create a uniform integer distribution within the desired range
            std::uniform_int_distribution<int> dist(min_value, max_value);

            int rand_pt_1 = dist(rng);
            //int rand_pt_1 = rand() % path_size;

            //int path_size = greedy_result.size();
            //int rand_pt_1 = rand() % path_size;
            std::uniform_int_distribution<int> dist2(0, path_size - rand_pt_1 - 1);
            
            int rand_pt_2 = rand_pt_1 + dist2(rng);
            //int rand_pt_2 = rand_pt_1 + (std::rand() % (path_size - rand_pt_1));

            std::uniform_int_distribution<int> dist3(0, rand_pt_2 - rand_pt_1);
            int shift = dist3(rng);
            //int shift = std::rand() % (rand_pt_2 - rand_pt_1 + 1);

            //std::cout << "Start: " << rand_pt_1 << ", End: " << rand_pt_2 << ", Shift: " << shift << std::endl;

            int section_size = rand_pt_2 - rand_pt_1 + 1;
            std::rotate(greedy_result.begin() + rand_pt_1, greedy_result.begin() + rand_pt_1 + shift, greedy_result.begin() + rand_pt_2 + 1);


            return greedy_result;
        }
    }
}


bool check_element_exist(std::vector<int> route, int element) {
	auto it = find(route.begin(), route.end(), element);
	// If element was found
	if (it != route.end()) {
	    //std::cout<<"true for:"<<element<<std::endl;
	    return true;
	}
        
    //the element is not in route 
	else {
	    //std::cout<<"false"<<std::endl;
	    return false;
	}
}

void getDeliveriesIntersectionIds(const std::vector<DeliveryInf>& deliveries){
    delivery_depot_ids.clear(); 

    for (int i = 0; i < deliveries.size(); i++) {

        // Check if intersection is repeated
        if (find(delivery_depot_ids.begin(), delivery_depot_ids.end(), deliveries[i].pickUp) != delivery_depot_ids.end()) {
            repeated_nodes[deliveries[i].pickUp].push_back(delivery_depot_ids.size());
        }

        if (find(delivery_depot_ids.begin(), delivery_depot_ids.end(), deliveries[i].dropOff) != delivery_depot_ids.end()) {
            repeated_nodes[deliveries[i].dropOff].push_back(delivery_depot_ids.size() + 1);
        }

        delivery_depot_ids.push_back(deliveries[i].pickUp);
        delivery_depot_ids.push_back(deliveries[i].dropOff);
    }

    return;
}

void computeMatrix(const float turn_penalty, int full_matrix_size, int delivery_matrix_size){
    travelTimeMatrix.clear();
    subPath.clear();

    travelTimeMatrix.resize(full_matrix_size);
    subPath.resize(full_matrix_size);
    
    #pragma omp parallel for
    for (int row = 0; row < full_matrix_size; row++) {
        SSMD_Dijkstra(row, delivery_depot_ids[row],turn_penalty, delivery_matrix_size, delivery_depot_ids, repeated_nodes);          
    }

    return;
}

int if_intersection_found(IntersectionIdx id, int total_destinations, std::vector<IntersectionIdx> important_ids){
    for (int col = 0; col < total_destinations; col++) {
        if (id == important_ids[col]) {
            return col;   
        }
    }
    return -1;
}

void SSMD_Dijkstra(int row, IntersectionIdx start_intersection, const float turn_penalty, int delivery_matrix_size, std::vector<IntersectionIdx> important_ids, std::unordered_map<IntersectionIdx, std::vector<int>> repeated_ids) {
    int total_destinations;

    if (row < delivery_matrix_size) {
        total_destinations = important_ids.size();
    } else {
        total_destinations = delivery_matrix_size;
    }
    
    int traversed = 0;
    std::vector<double> travel_time;
    std::vector<std::vector<StreetSegmentIdx>> sub_path_vec;

    travel_time.resize(total_destinations, INFINITY);
    sub_path_vec.resize(total_destinations);

    std::priority_queue<Node> pq;
    std::vector<SegTime> seg_times(getNumIntersections(), SegTime(INFINITY));

    pq.push(Node(0, start_intersection, 0.0, 0.0, -1));

    
    while (!pq.empty()) {
        Node current = pq.top();
        pq.pop();

        if (current.real_time < seg_times[current.name].best_time) {
            seg_times[current.name].best_time = current.real_time;
            
            if (current.parent != -1) {
                seg_times[current.name].prevIntersection = current.parent;
                seg_times[current.name].seg = current.seg;
            }
            
            //////////////////////////
            std::vector<StreetSegmentIdx> path;
            IntersectionIdx node_id;
            IntersectionIdx prev_id;
            int col = if_intersection_found(current.name, total_destinations, important_ids);

            // intersection found, reconstruct path:
            if (col != -1) {
                if (travel_time[col] == INFINITY) {
                    traversed++;

                    if (!repeated_ids[current.name].empty()) {
                        for (int i : repeated_ids[current.name]) {
                            traversed++;
                        }
                    }
                }

                travel_time[col] = seg_times[current.name].best_time;

                if (!repeated_ids[current.name].empty()) {
                    for (int i : repeated_ids[current.name]) {
                        travel_time[i] = seg_times[current.name].best_time;
                    }
                }

                node_id = current.name;
                prev_id = seg_times[node_id].prevIntersection;

                while (node_id != start_intersection) {
                    path.insert(path.begin(), seg_times[node_id].seg);
                    node_id = prev_id;
                    prev_id = seg_times[node_id].prevIntersection;
                }
                
                sub_path_vec[col] = path;

                if (!repeated_ids[current.name].empty()) {
                    for (int i : repeated_ids[current.name]) {
                        sub_path_vec[i] = path;
                    }
                }
                
                if (traversed == total_destinations) {
                    travelTimeMatrix[row] = travel_time;
                    subPath[row] = sub_path_vec;

                    return;
                }  
            }
           
            //////////////////////////
            std::vector<StreetSegmentIdx> current_seg = findSegments(current.name);
            int total_segments = current_seg.size();
            std::vector<Node> private_nodes;
            private_nodes.resize(total_segments);

            #pragma omp parallel for
            for (int i = 0; i < total_segments; i++) {
                StreetSegmentInfo current_info = getStreetSegmentInfo(current_seg[i]);
                StreetIdx current_street = current_info.streetID;

                if (!current_info.oneWay || (current_info.oneWay && current_info.from == current.name)) {

                    IntersectionIdx next_intersection;
                    if (current_info.to == current.name) {
                        next_intersection = current_info.from;
                    } else {
                        next_intersection = current_info.to;
                    }
                    
                    StreetSegmentIdx previous_seg;
                    StreetSegmentInfo previous_info;
                    StreetIdx previous_street;

                    double penalty = 0.0;

                    if (current.parent != -1) {
                        previous_seg = seg_times[current.name].seg;
                        previous_info = getStreetSegmentInfo(previous_seg);
                        previous_street = previous_info.streetID;
                        
                        if (current_street != previous_street) {
                            penalty = penalty + turn_penalty;
                        }
                    }
                    
                    double travel_time = findStreetSegmentLength(current_seg[i])/current_info.speedLimit;
                    double new_time = current.real_time + travel_time + penalty;      

                    private_nodes[i] = Node(current_seg[i], next_intersection, new_time, new_time, current.name);
                }
            }

            for (int i = 0; i < total_segments; i++) {
                if (private_nodes[i].seg != -1) {
                    pq.push(private_nodes[i]);
                }
            }
        }
    }
}