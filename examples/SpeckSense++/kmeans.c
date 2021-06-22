#include "contiki.h"
#include "kmeans.h"
#include <math.h>

/*Thresholds, could be in a Makefile for "UI" access*/
#define INTERFERENCE_POWER_LEVEL_THRESHOLD 11
#define INTERFERENCE_DURATION_PROACTIVE 900
#define INTERFERENCE_DURATION_REACTIVE 20
#define INTERFERENCE_DURATION_RANDOM 150
#define INTERFERENCE_NUMBER_SAMPLES 5
#define INTERFERENCE_NUMBER_THRESHOLD 10

#define INTERFERENCE_DURATION_BLUETOOTH_MIN 100
#define INTERFERENCE_DURATION_BLUETOOTH_MAX 500
#define INTERFERENCE_POWER_LEVEL_BLUETOOTH 4

#define INTERFERENCE_DURATION_WIFI_MIN 450
#define INTERFERENCE_DURATION_WIFI_MAX 800
#define INTERFERENCE_POWER_LEVEL_WIFI 6

#define EUCLIDEAN_DISTANCE_WEIGHT 5000

#define DEBUG_MODE 1

/*Debug colors*/

/*fonts color*/
#define FBLACK "\033[30;"
#define FRED "\033[31;"
#define FGREEN "\033[32;"
#define FYELLOW "\033[33;"
#define FBLUE "\033[34;"
#define FPURPLE "\033[35;"
#define D_FGREEN "\033[6;"
#define FWHITE "\033[7;"
#define FCYAN "\x1b[36m"
/*background color*/
#define BBLACK "40m"
#define BRED "41m"
#define BGREEN "42m"
#define BYELLOW "43m"
#define BBLUE "44m"
#define BPURPLE "45m"
#define D_BGREEN "46m"
#define BWHITE "47m"
/*end color*/
#define NONE "\033[0m"


/*Global variables*/
static int X[RUN_LENGTH][2];

static int K[10][2];
static uint32_t cost;
static int num_vectors;

static int cluster_test, cluster_pref;
static uint8_t vector_label[RUN_LENGTH];
static uint16_t record_ptr,
    plevel,
    j;

static uint32_t cost_final, 
    prev_cost_final,
    diffcost_between_clusters,
    cost_runs,
    old_cost,
    cost_diff,
    sq_distance, min_distance,
    distance,
    free_samples,
    busy_samples;
static int prev_K_final[10][2], K_final[10][2];
static int i, iter = 1, num_clusters_final = 0,
              prev_num_clusters_final = 0,
              nruns = 0, idx;

static unsigned sum_points1 = 0, 
    sum_points2 = 0, 
    num_points = 0;

static int stop_threshold = 1;

static int suspicion_level = 0;
static int suspicion_arr_cnt = 0;
static int suspicion_vector_array[5];
static uint16_t PCJ_cnt = 0;
static uint16_t RSJ_cnt = 0;

/*In order to profile the different interference*/
/*Perhaps create a debug file that has this variables ? or atleast a function*/
static int profiling_nr_DUR;
static int profiling_tot_DUR;
static int profiling_amount_of_times_DUR;

static int profiling_nr_PL;
static int profiling_tot_PL;
static int profiling_amount_of_times_PL;

static int accuracy_ratio = 0;
static int accuracy_suc = 0; 
static int accuracy_amount_of_times = 0;

static struct cluster_info
{
    uint16_t /*float*/ vector_duration;
    uint8_t /*float*/ plevel;
} clusters[10];

/*---------------------------------------------------------------------------*/
uint16_t abs_diff(uint16_t a, uint16_t b)
{
    if (a <= b){
        return b - a;
    }else{
        return a - b;
    }
}
/*---------------------------------------------------------------------------*/
void print_cluster_centroids(void)
{
    for (int i = 0; i < prev_num_clusters_final; i++) {
        printf("cluster %d : vector_duration: %d : plevel: %d \n", i, clusters[i].vector_duration, clusters[i].plevel);
    }
}
/*---------------------------------------------------------------------------*/

/*Can be moved to another class file ?*/
/*---------------------------------------------------------------------------*/
void swap(struct cluster_info *xp, struct cluster_info *yp)
{
    struct cluster_info temp = *xp;
    *xp = *yp;
    *yp = temp;
}
/*---------------------------------------------------------------------------*/
void bubble_sort(struct cluster_info arr[], int n, int after_size)
{
    int i, j;
    for (i = 0; i < n - 1; i++){
        for (j = 0; j < n - i - 1; j++){

            if (after_size){
                if (arr[j].vector_duration < arr[j + 1].vector_duration){
                    swap(&arr[j], &arr[j + 1]);
                }
            }else{
                if (arr[j].plevel < arr[j + 1].plevel){
                    swap(&arr[j], &arr[j + 1]);
                }
            }
        }
    }
}
/*---------------------------------------------------------------------------*/
void calc_profiling(void)
{
    profiling_tot_DUR += clusters[0].vector_duration;
    profiling_amount_of_times_DUR += 1;
    profiling_nr_DUR = profiling_tot_DUR / profiling_amount_of_times_DUR;

    profiling_tot_PL += clusters[0].plevel;
    profiling_amount_of_times_PL += 1;
    profiling_nr_PL = profiling_tot_PL / profiling_amount_of_times_PL;
    printf("--- profiling start ---\n");
    printf("ptD: %d, paoftD: %d, pnD: %d, ptPL: %d, paotPL: %d, pnPL: %d\n", profiling_tot_DUR, profiling_amount_of_times_DUR, profiling_nr_DUR, profiling_tot_PL, profiling_amount_of_times_PL, profiling_nr_PL);
    printf("--- profiling end ---\n");
}
/*---------------------------------------------------------------------------*/
void calc_accuracy(void)
{
    accuracy_suc++;
    accuracy_ratio = accuracy_suc / accuracy_amount_of_times;
    printf("Accuracy suc: %d, Accuracy nr time: %d, Accuracy ratio: %d percentage\n", accuracy_suc, accuracy_amount_of_times, accuracy_ratio);
}
/*---------------------------------------------------------------------------*/
void calc_consistency(void)
{
    if (suspicion_arr_cnt >= 5){
        printf("suspecious arr cnt: %d\n", suspicion_arr_cnt);
        printf("Suspecious_level is 10 \n");
        if (suspicion_arr_cnt >= INTERFERENCE_NUMBER_SAMPLES){

            int supicious_median = 0;
            for (int i = 0; i < INTERFERENCE_NUMBER_SAMPLES; i++){
                printf("Suspecious values: %d\n", suspicion_vector_array[i]);
                supicious_median += suspicion_vector_array[i];
            }
            supicious_median /= INTERFERENCE_NUMBER_SAMPLES;
            printf("Median is: %d\n", supicious_median);
            for (int i = 0; i < INTERFERENCE_NUMBER_SAMPLES; i++){
                printf("Difference: %d >= Threashold: %d\n", abs(supicious_median - suspicion_vector_array[i]), INTERFERENCE_DURATION_RANDOM);
                if (abs(supicious_median - suspicion_vector_array[i]) >= INTERFERENCE_DURATION_RANDOM){
                    printf(" \n");
                    printf("THE REAL PROACTIVE RANDOM INTERFERENCE JAMMER\n");
                    printf(" \n");
                    suspicion_level = 0;
                    break;
                }
            }
            suspicion_arr_cnt = 0;
        }

        if (PCJ_cnt >= 1){
            printf("CONSTANT JAMMER sus : %d\n", PCJ_cnt);
        }
        else if (RSJ_cnt >= 1){
            printf("REACTIVE JAMMER sus\n");
        }
        else{
            printf("UNKNOWN JAMMER or FALSE ALARM\n");
        }

        PCJ_cnt = 0;
        RSJ_cnt = 0;
        suspicion_level = 0;
    }else{
        printf("\n");
        printf("Nothing\n");
        printf("\n");

        /*Increase the suspicion level if RSJ or PCJ has occured earlier*/
        if (RSJ_cnt >= 1 || PCJ_cnt >= 1){
            suspicion_level += 1;
        }
    }
}
/*---------------------------------------------------------------------------*/
void check_similarity(int profiling)
{

    accuracy_amount_of_times++;
    /*Sort the size after highest duration (Bubblesort)*/
    bubble_sort(clusters, prev_num_clusters_final, 1); /*Sort after size*/
    bubble_sort(clusters, prev_num_clusters_final, 0); /*Acutally better to sort after power level*/

    if(profiling){
        calc_profiling();
    }


    for (int i = 0; i < prev_num_clusters_final; i++){
        /*Debug*/
        if (DEBUG_MODE){
            if (clusters[i].plevel >= 6){
                printf("cluster %d : vector_duration: %d :", i, clusters[i].vector_duration);
                printf(D_FGREEN BBLUE "plevel: " NONE);
                printf("%d ", clusters[i].plevel);
                printf("num_vectors: %d\n", num_vectors);
            }else{
                printf("cluster %d : vector_duration: %d : plevel: %d num_vectors: %d\n", i, clusters[i].vector_duration, clusters[i].plevel, num_vectors);
            }
        }

        if (clusters[i].vector_duration >= INTERFERENCE_DURATION_PROACTIVE && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_THRESHOLD){
            printf(" \n");
            printf("THE REAL PROACTIVE CONSTANT INTERFERENCE JAMMER\n");
            printf(" \n");

            suspicion_vector_array[suspicion_arr_cnt] = clusters[i].vector_duration;
            suspicion_arr_cnt++;
            suspicion_level += 2;
            PCJ_cnt += 1;
            break;
        }
        if (clusters[i].vector_duration >= 15 && clusters[i].vector_duration <= 30 && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_THRESHOLD){

            printf(" \n");
            printf("THE REAL SFD REACTIVE INTERFERENCE JAMMER\n");
            printf(" \n");
            suspicion_vector_array[suspicion_arr_cnt] = clusters[i].vector_duration;
            suspicion_arr_cnt++;
            suspicion_level += 2;
            RSJ_cnt += 1;

            break;
        }
        if (clusters[i].vector_duration >= INTERFERENCE_DURATION_BLUETOOTH_MIN && clusters[i].vector_duration <= (INTERFERENCE_DURATION_BLUETOOTH_MAX)
            && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_BLUETOOTH && clusters[i].plevel <= INTERFERENCE_POWER_LEVEL_BLUETOOTH + 1){
            printf(" \n");
            printf("BLUETOOTH INTERFERENCE\n");
            printf(" \n");
            break;
        }
        if (clusters[i].vector_duration >= INTERFERENCE_DURATION_WIFI_MIN && clusters[i].vector_duration <= (INTERFERENCE_DURATION_BLUETOOTH_MAX)
            && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_WIFI && clusters[i].plevel <= INTERFERENCE_POWER_LEVEL_WIFI + 1){
            printf(" \n");
            printf("WiFi INTERFERENCE\n");
            printf(" \n");
            break;
        }
    }

    calc_consistency();
}
/*---------------------------------------------------------------------------*/
int kmeans(struct record *record, int rle_ptr)
{
    /* Initialize all variables */
    cost = 0;
    cluster_test = 0;
    cluster_pref = -1;
    record_ptr = 0;
    plevel = 0;
    j = 0, num_vectors = 0;

    cost_final = 65535;
    prev_cost_final = 65535;
    diffcost_between_clusters = 65535;
    cost_runs = 65535;
    iter = 1;
    num_clusters_final = 0;
    prev_num_clusters_final = 0;
    nruns = 0;
    old_cost = 65535;
    cost_diff = 0;

    for (i = 0; i < 10; i++){
        prev_K_final[i][0] = 0;
        prev_K_final[i][1] = 0;
        K_final[i][0] = 0;
        K_final[i][1] = 0;
    }

    free_samples = 0;
    busy_samples = 0;
    if (rle_ptr == RUN_LENGTH)
        rle_ptr--;

    printf("------\n");

    /*Copy pointer value to X array*/
    for (int i = 0; i < RUN_LENGTH; i++){
        X[i][0] = record->rssi_rle[i][1];
        X[i][1] = record->rssi_rle[i][0];
    }

    num_vectors = RUN_LENGTH;

    cluster_test = 1;
    while ((cluster_test < 11) && (diffcost_between_clusters > stop_threshold)){
        cost_runs = 65535;
        nruns = 0;
        cluster_test++;
        if (cluster_test > 1){
            for (i = 0; i < num_clusters_final; i++){
                prev_num_clusters_final = num_clusters_final;
                prev_K_final[i][0] = K_final[i][0];
                prev_K_final[i][1] = K_final[i][1];
                prev_cost_final = cost_final;
            }
        }

        while (nruns < 5){
            old_cost = 65535;
            cost_diff = 0;
            iter = 1;

            if (old_cost > cost)
            {
                cost_diff = old_cost - cost;
            }else{
                cost_diff = cost - old_cost;
            }

            /* Initialize clusters
            */
            for (i = 0; i < 10; i++){
                K[i][0] = 0;
                K[i][1] = 0;
                clusters[i].vector_duration = 0;
                clusters[i].plevel = 0;
            }

            /* We randomly initialize clusters to points in X */
            if (cluster_pref >= 0){
                K[0][0] = X[cluster_pref][0];
                K[0][1] = X[cluster_pref][1];
                for (i = 1; i < cluster_test; i++){
                    idx = random_rand() % (RUN_LENGTH); //num_vectors;
                    K[i][0] = X[idx][0];                
                    K[i][1] = X[idx][1];                
                }
            }
            else{

                for (i = 0; i < cluster_test; i++){
                    idx = random_rand() % (RUN_LENGTH); //num_vectors;
                    K[i][0] = X[idx][0];
                    K[i][1] = X[idx][1];
                }
            }

            /* Repeat until desired number of iterations 
            * OR the cost_diff is smaller than a 
            * randomly chosen value
            */
            while ((iter < 11) /*11*/ && (cost_diff > stop_threshold)){

                if (iter > 1){
                    old_cost = cost;
                }

                /* Move centroid to the average 
                * of all related feature points
                */
                cost = 0;
                for (i = 0; i < num_vectors; i++){
                    sq_distance = 65535;
                    for (j = 0; j < cluster_test; j++){
                        distance = ((K[j][0] - X[i][0]) * (K[j][0] - X[i][0])) +
                                   ((K[j][1] - X[i][1]) * (K[j][1] - X[i][1])) * EUCLIDEAN_DISTANCE_WEIGHT;

                        if ((j == 0) || (distance < sq_distance)){
                            sq_distance = distance;
                            vector_label[i] = j;
                        }
                    }
                    cost = cost + sq_distance;
                }

                for (j = 0; j < cluster_test; j++){
                    sum_points1 = 0;
                    sum_points2 = 0;
                    num_points = 0;
                    for (i = 0; i < num_vectors; i++){
                        if (vector_label[i] == j){
                            sum_points1 = sum_points1 + X[i][0];
                            sum_points2 = sum_points2 + X[i][1];
                            num_points++;
                        }
                    }

                    if (num_points){
                        K[j][0] = sum_points1 / num_points;
                        K[j][1] = sum_points2 / num_points;
                    }else{
                        K[j][0] = 0x7FFF;
                        K[j][1] = 0x7FFF;
                    }
                }

                cost /= num_vectors;

                /* Added an enhancement to the clustering algorithm. 
                * Here, instead of randomly selecting cluster points in 
                * the next iteration, we explicitly choose burst features 
                * that have a large cost, i.e. they have a large distance 
                * from their cluster head in the current iteration.
                * This should hopefully solve the problem of outlier features 
                * not being classified separately.
                */
                min_distance = 0;
                for (i = 0; i < num_vectors; i++){
                    sq_distance = ((K[vector_label[i]][0] - X[i][0]) * (K[vector_label[i]][0] - X[i][0])) +
                                  ((K[vector_label[i]][1] - X[i][1]) * (K[vector_label[i]][1] - X[i][1])) * EUCLIDEAN_DISTANCE_WEIGHT;

                    if (sq_distance > min_distance){
                        min_distance = sq_distance;
                        cluster_pref = i;
                    }
                }

                if (old_cost > cost)
                {
                    cost_diff = old_cost - cost;
                }else{
                    cost_diff = cost - old_cost;
                }

                iter++;
            }

            if (cost_runs > cost){
                cost_runs = cost;
                num_clusters_final = 0;

                for (i = 0; i < cluster_test; i++){

                    /*Increase the K if the powerlevel or duration doesn't equal 32767*/
                    if (K[i][0] != 0x7FFF && K[i][1] != 0x7FFF) {
                        K_final[num_clusters_final][0] = K[i][0];
                        K_final[num_clusters_final][1] = K[i][1];
                        num_clusters_final++;
                    }
                }
            }

            nruns++;
        }

        if (cost_final > cost_runs){
            diffcost_between_clusters = cost_final - cost_runs;
            cost_final = cost_runs;
        }else{
            diffcost_between_clusters = cost_runs - cost_final;
            cost_final = cost_runs;
        }
    }

    if (num_clusters_final >= 1){
        for (i = 0; i < prev_num_clusters_final; i++) {
            clusters[i].vector_duration = prev_K_final[i][0];
            clusters[i].plevel = prev_K_final[i][1];

            
            if (DEBUG_MODE){
                if (clusters[i].plevel >= 6){
                    printf("cluster %d : vector_duration: %d :", i, clusters[i].vector_duration);
                    printf(D_FGREEN BBLUE "plevel: " NONE);
                    printf("%d ", clusters[i].plevel);
                    printf("num_vectors: %d\n", num_vectors);
                }else{
                    printf("cluster %d : vector_duration: %d : plevel: %d num_vectors: %d\n", i, clusters[i].vector_duration, clusters[i].plevel, num_vectors);
                }
            }
        }
    }

    return prev_num_clusters_final;
}
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/*Used in process_ID == 2 */
float channel_metric_rssi_threshold(struct record *record, int rle_ptr)
{
    uint16_t free_samples = 0;
    busy_samples = 0;
    record_ptr = 0;
    if (rle_ptr == RUN_LENGTH)
        rle_ptr--;
    /* Populate feature vectors for the clustering process */
    while (record_ptr < rle_ptr)
    {
        /* Process burst size and power level for a 
        * continuous stretch of RSSI readings 
        * above -90 dBm ( > power level 1)
        */
        if (record->rssi_rle[record_ptr][0] <= 140) //POWER_LEVELS
        {
            while ((record->rssi_rle[record_ptr][0] > 1) &&
                   (record_ptr < rle_ptr))
            {
                busy_samples += record->rssi_rle[record_ptr][1];
                record_ptr++;
            }

            /* We now see a burst of RSSI values 
            * below -90 dBm (power level 1). 
            * Update timestamps and free samples
            */
            if (record_ptr < rle_ptr)
            {
                free_samples += record->rssi_rle[record_ptr][1];
                record_ptr++;
            }
        }
        else
            record_ptr++;
    }

    printf("free samples = %d, busy samples = %ld\n",
           free_samples, busy_samples);
    return (((float)(free_samples * 1.0)) / (free_samples + busy_samples));
}
/*---------------------------------------------------------------------------*/
