#include <stdio.h>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include <cmath>
#include <vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <random>
#include <iostream>
#include <fstream>

using namespace std;

#define MAX_SUCCESS 5
#define MAX_IT 200
#define FREEZING 0.1
#define MARKOV_CHAIN 20

// otsu_args of threads
struct OtsuArgs
{
    int x_min;
    int x_max;
    vector<pair<int, float>> population;
    int n_it;
    float temp;
};

float objectiveFunc(int threshold);
int neighborSearch(int threshold, float temp, int space_min, int space_max);
void *simu_otsu(void * otsu_args);
cv::Mat *segmentation(cv::Mat * img_input, int threshold);
float *compute_freq(cv::Mat * img_input);
bool write_data(OtsuArgs *arg1, pair<int,float> best, int n_it, float temp, double time_r);

/* Frequency of pixel */
float *freq_pixel;

int main(int argc, char const *argv[])
{
    if (argc != 2){
        printf("Error invalid parammeter!!\n");
        return -1;
    }

    /* Read input image */
    cv::Mat img_input = cv::imread(argv[1], CV_LOAD_IMAGE_GRAYSCALE);

    if (!img_input.data){
        printf("Error image not found!!\n");
        return -1;
    }

    printf("image path: %s\nimage size: %dx%d\n", argv[1], img_input.rows, img_input.cols);

    // Compute pixel frequency
    clock_t tStart = clock();
    freq_pixel = compute_freq(&img_input);

    // create struct data of thread1
    OtsuArgs otsu_arg1;
    otsu_arg1.population.clear();
    otsu_arg1.x_min = 0;
    otsu_arg1.x_max = 127;
    otsu_arg1.n_it = 0;

    simu_otsu((void*) &otsu_arg1);
    double runtime = (double)(clock() - tStart)/CLOCKS_PER_SEC;

    // get threads solutions
    pair<int, float> thresh_thread1 = otsu_arg1.population[otsu_arg1.population.size()-1];

    int n_it;
    float temp;
    n_it = otsu_arg1.n_it;
    temp = otsu_arg1.temp;

    // show best solution
    printf("\nTime running: %.4fs\n", runtime);
    printf("best threshold : %d -> %f\n", thresh_thread1.first, thresh_thread1.second);

    // create new image for segmentation
    cv::Mat * img_output = segmentation(&img_input, thresh_thread1.first);
    
    // create thread of showing output image
    cv::namedWindow( "Original image", cv::WINDOW_AUTOSIZE );  // Create a window for display.
    cv::imshow( "Original image", img_input );
    cv::imshow( "Segmented image", *img_output ); // Show our image inside it.
    cv::waitKey(0); // Wait for a keystroke in the window


    write_data(&otsu_arg1,  thresh_thread1, n_it, temp, runtime);
    printf("writed data...\n");

    return 0;
}

bool write_data(OtsuArgs *arg1, pair<int,float> best, int n_it, float temp, double time_r)
{
    fstream myfile;
    myfile.open("parallel_annealing/data/population1.csv", std::fstream::out);
    myfile << "threshold;energy" << endl;
    for (auto it: arg1->population)
        myfile << it.first << ";" << it.second << ";" << endl;
    myfile.close();

    myfile.open("parallel_annealing/data/bests.csv", std::fstream::app);
    myfile << best.first << ";" << best.second << ";" << n_it << ';' << temp << ';' << time_r << ";" << endl;
    myfile.close();

    return true;
}

float *compute_freq(cv::Mat * img_input)
{
    float *freq = (float*) calloc(256, sizeof(int));
    int num_pixel = img_input->rows*img_input->cols;

    // compute pixel frequency
    for (int i=0; i < img_input->rows; i++){
        for (int j=0; j < img_input->cols; j++){
            int pixel_value = (int)img_input->at<uchar>(i,j);
            freq[pixel_value]++;
        }
    }

    // compute pixel probability 
    for (int i=0; i < 256; i++)
        freq[i] = (float)freq[i]/num_pixel;

    return freq;
}

cv::Mat *segmentation(cv::Mat * img_input, int threshold)
{
    cv::Mat *img_output = new cv::Mat(img_input->rows, img_input->cols, CV_8UC1, cv::Scalar(70));
    for (int i = 0; i < img_input->cols; i++){
        for (int j = 0; j < img_input->rows; j++){
            int pixel_value = (int) img_input->at<uchar>(i,j);

            if (pixel_value < threshold)
                img_output->at<uchar>(i,j) = 0;
            else
                img_output->at<uchar>(i,j) = 255;
        }
    }

    return img_output;
}


float objectiveFunc(int threshold)
{
    float w0 = 0, w1 = 0, u0 = 0, u1 = 0;

    // Objective probability (frequency of target)
    for (int i=0; i < threshold; i++)
        w0 += freq_pixel[i];

    // background part probability (frequency of background)
    for (int i=threshold; i <= 255; i++)
        w1 += freq_pixel[i];

    // average value of the target part
    if (w0 == 0)
        u0 = 0;
    else
        for (int i=0; i < threshold; i++)
            u0 += (i*freq_pixel[i])/w0;

    // average value of background part
    if (w1 == 0)
        u1 = 0;
    else
        for (int i=threshold; i < 255; i++)
            u1 += (i*freq_pixel[i])/w1;

    // variance of region target and region background
    return w0*w1*pow(u0-u1, 2);
}

int neighborSearch(int threshold, float temp, int space_min, int space_max)
{    
    // define min range of neighbor search
    int min = round(threshold-temp);
    if (min < space_min || min < 0){
        min = space_min;
    }
    
    // define max range of neighbor search
    int max = round(threshold+temp);
    if (max > space_max || max > 255){
        max = space_max;
    }
    
    // generate random number
    random_device rd;
    return rd()%max + min;
}


void *simu_otsu(void * otsu_args)
{
    OtsuArgs *args = (OtsuArgs*) otsu_args;

    // get initial threshold and your variancy
    srand(time(NULL));
    int threshold = rand()%(args->x_max-args->x_min + 1) + args->x_min;
    float energy = objectiveFunc(threshold);

    // set initial temperature
    float temp = 100;

    // Loop control
    int current_it = 0, n_success;

    do
    {
        n_success = 0;

        // Markov chains
        for (int i = 0; i < MARKOV_CHAIN && n_success < MAX_SUCCESS; i++)
        {
            // pick a random neighbour
            int new_threshold = neighborSearch(threshold, temp, args->x_min, args->x_max);
            float new_energy = objectiveFunc(new_threshold);

            // compute energy variance and generate random number
            random_device rd;
            float rand_num = (float) rd()/rd.max();
            float delt_energy = new_energy - energy;

            // Test new solution
            if (delt_energy > 0 || exp(delt_energy/temp) > rand_num){
                threshold = new_threshold;
                energy = new_energy;
                args->population.push_back(make_pair(threshold,energy));
                n_success++;
            }
        }
        temp *= 0.98;
        current_it++;
    } while (n_success > 0 && current_it < MAX_IT && temp > FREEZING);
    args->n_it = current_it;
    args->temp = temp;
    /* printf("n_success: %d - current_it: %d - temp: %f\n", n_success, current_it, temp);
    printf("end threshold: %d\nend energy: %f\n", threshold, energy); */

    return NULL;
}
