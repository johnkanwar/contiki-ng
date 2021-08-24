#include "contiki.h"
#include "kmeans.h"
#include <math.h>

/*Thresholds, could be in a Makefile for "UI" access*/
#define INTERFERENCE_POWER_LEVEL_THRESHOLD 16       //11
#define INTERFERENCE_DURATION_PROACTIVE 900
#define INTERFERENCE_DURATION_REACTIVE 20
#define INTERFERENCE_DURATION_RANDOM 150
#define INTERFERENCE_NUMBER_SAMPLES 5
#define INTERFERENCE_NUMBER_THRESHOLD 10

#define INTERFERENCE_DURATION_SFD_MIN 15
#define INTERFERENCE_DURATION_SFD_MAX 30

// #define INTERFERENCE_DURATION_BLUETOOTH_MIN 14
// #define INTERFERENCE_DURATION_BLUETOOTH_MAX 26
// #define INTERFERENCE_POWER_LEVEL_BLUETOOTH 14

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

/*--------------------------*/
#define TVARIANCE 7
#define TVARIANCE_CUMULATIVE 30

static uint32_t relative_ts;
static uint16_t bsize;
static uint8_t num_bursts, num_occurrences, burst_interval[RUN_LENGTH>>1];;
static uint32_t burst_ts[RUN_LENGTH>>1];
static uint8_t  burst_label[RUN_LENGTH>>1], max_cnt,intf_idx, wifi_beacon;
static uint16_t tlist[15][2];
static uint32_t mean_period;

static int profiling_inter_bursts_runs;
static int profiling_inter_bursts_tot;
static int profiling_inter_bursts_mean;

static int profiling_clusters_runs;
static int profiling_clusters_tot;
static int profiling_clusters_mean;
/*--------------------------*/

static int run_old_specksense = 0;
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

static struct cluster_info_old {
     uint32_t mean_period;
     uint16_t burst_size;
     uint8_t plevel;     
     uint8_t num_occurrences;
     uint8_t intf_type;
} clusters_old[10];

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

//  Normal air
//  ptD: 11422, paoftD: 1283, pnD: 8, ptPL: 14585, paotPL: 1283, pnPL: 11         │Nothing
//  power level 11
//  duration 8

// Bluetooth
// ptD: 13173, paoftD: 502, pnD: 26, ptPL: 7337, paotPL: 502, pnPL: 14           │ START 
// power level 14
// duration 26


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

            int suspicious_median = 0;
            for (int i = 0; i < INTERFERENCE_NUMBER_SAMPLES; i++){
                printf("Suspecious values: %d\n", suspicion_vector_array[i]);
                suspicious_median += suspicion_vector_array[i];
            }
            suspicious_median /= INTERFERENCE_NUMBER_SAMPLES;
            printf("Median is: %d\n", suspicious_median);
            for (int i = 0; i < INTERFERENCE_NUMBER_SAMPLES; i++){
                printf("Difference: %d >= Threashold: %d\n", abs(suspicious_median - suspicion_vector_array[i]), INTERFERENCE_DURATION_RANDOM);
                if (abs(suspicious_median - suspicion_vector_array[i]) >= INTERFERENCE_DURATION_RANDOM){
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
        } else
        if (clusters[i].vector_duration >= INTERFERENCE_DURATION_SFD_MIN && clusters[i].vector_duration <= INTERFERENCE_DURATION_SFD_MAX 
            && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_THRESHOLD){

            printf(" \n");
            printf("THE REAL SFD REACTIVE INTERFERENCE JAMMER\n");
            printf(" \n");
            suspicion_vector_array[suspicion_arr_cnt] = clusters[i].vector_duration;
            suspicion_arr_cnt++;
            suspicion_level += 2;
            RSJ_cnt += 1;

            break;
        } else
        if (clusters[i].vector_duration >= INTERFERENCE_DURATION_BLUETOOTH_MIN && clusters[i].vector_duration <= (INTERFERENCE_DURATION_BLUETOOTH_MAX)
            && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_BLUETOOTH && clusters[i].plevel <= INTERFERENCE_POWER_LEVEL_BLUETOOTH + 1){
            printf(" \n");
            printf("BLUETOOTH INTERFERENCE\n");
            printf(" \n");
            run_old_specksense = 1;
            break;
        } else
        if (clusters[i].vector_duration >= INTERFERENCE_DURATION_WIFI_MIN && clusters[i].vector_duration <= (INTERFERENCE_DURATION_BLUETOOTH_MAX)
            && clusters[i].plevel >= INTERFERENCE_POWER_LEVEL_WIFI && clusters[i].plevel <= INTERFERENCE_POWER_LEVEL_WIFI + 1){
            printf(" \n");
            printf("WiFi INTERFERENCE\n");
            printf(" \n");
            run_old_specksense = 1;
            break;
        }
        else
        {
            run_old_specksense = 1;
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







/*-------------- START This is for classification --------------*/

void channel_rate(struct record *record, int n_clusters)
{
      record_ptr = 0;
      free_samples = 0;
      busy_samples = 0;
      wifi_beacon = 0;
      
      for (i = 0; i < n_clusters; i++)
	   if (clusters_old[i].intf_type == WIFI_BEACON)
		wifi_beacon = 1;
	   
      while(record_ptr < RUN_LENGTH)
      {	
	  if (record->rssi_rle[record_ptr][0] == 1)
	      free_samples = free_samples + record->rssi_rle[record_ptr][1];
	  else
	      busy_samples = busy_samples + record->rssi_rle[record_ptr][1];
	  
	  record_ptr++;
      }

      printf("WiFi-%d: Clusters-%d: Cost-%ld: fs-%lu: ts-%lu: ",
			    wifi_beacon, n_clusters, prev_cost_final,
			    free_samples, (free_samples+busy_samples));
}

void update_tlist(uint16_t ts_diff) {
    uint8_t i, match_found = 0;
    
    for (i = 0; i < 15; i++) {
        if (!tlist[i][0] && !match_found) {
            tlist[i][0] = ts_diff;
            tlist[i][1] = 1;
            break;
        }
        if ((ts_diff % tlist[i][0] < TVARIANCE_CUMULATIVE) &&
           (ts_diff > (tlist[i][0] >> 1))){
            match_found = 1;
            tlist[i][1]++;
        }
        else if ( (tlist[i][0] - (ts_diff % tlist[i][0])) 
                    < TVARIANCE_CUMULATIVE) {
            match_found = 1;
            tlist[i][1]++;
        }
    }    
}

void add_to_tlist(uint16_t ts_diff) {
    uint8_t i;
    for (i = 0; i < 15; i++) {
        if (!tlist[i][0]) {
            tlist[i][0] = ts_diff;
            tlist[i][1] = 1;
            break;
        }
        
        if (ts_diff % tlist[i][0] < TVARIANCE) {
            if (abs_diff(ts_diff, tlist[i][0]) 
                    < (tlist[i][0] >> 1)) {
                tlist[i][0] = (tlist[i][0] + ts_diff) >> 1;
                break;
            }
        }
        else if ( (tlist[i][0] - (ts_diff % tlist[i][0])) 
                    < TVARIANCE) {
            if (abs_diff(ts_diff, tlist[i][0]) 
                    < (tlist[i][0] >> 1)) {
                tlist[i][0] = (tlist[i][0] + ts_diff) >> 1;            
                break;
            }
        }
    }    
}

//This is where it happens
void check_periodic(uint16_t itr_cnt, int cluster_id) {
    uint32_t /*sq_term = 0,*/ sum_term = 0, mean_t;
    uint16_t prev_burst = 0, number_samples, confidence = 0, pos = 0;   
    uint16_t sum_diff = 0;
    uint8_t j = 0, n_sources = 0;
    
    /* Calculate for every burst, the interburst_separation 
     * from its previous burst instance    
     */
    for (i = 0; i < 15; i++) {
        tlist[i][0] = 0;
        tlist[i][1] = 0;
    }

    num_occurrences = 0; 
    for (i = 0; i < num_bursts; i++)
    {
        if (burst_label[i] == cluster_id)
        {
            if (num_occurrences) {               
                burst_interval[num_occurrences-1] = (uint16_t) (burst_ts[i] 
                                                    - burst_ts[prev_burst]);
                add_to_tlist((uint16_t) (burst_ts[i] - burst_ts[prev_burst]));
            }
              
            prev_burst = i;
            num_occurrences++;
        }
    }
    
    for (i = 0; i < num_occurrences-1; i++) {
        sum_diff = burst_interval[i];
        sum_term += sum_diff;
        update_tlist(sum_diff);
        for (j = i+1; j < num_occurrences-1; j++) {
            sum_diff += burst_interval[j];
            update_tlist(sum_diff);
        }
    }
    
    // for(int i =0; i < 15; i++)
    // {
    //     printf("tlist: (%d, %d) \n",tlist[i][0],tlist[i][1]);
    // }
    // printf("\n");
    if (num_occurrences > 2) {
        mean_t = (uint32_t) sum_term/(num_occurrences - 1);
        printf("Channel %d: Kperiod(%d,%d): mean: %lu:", itr_cnt, clusters_old[cluster_id].burst_size, clusters_old[cluster_id].plevel, mean_t);

        profiling_inter_bursts_runs += clusters_old[cluster_id].burst_size;
        profiling_clusters_runs += clusters_old[cluster_id].plevel;

        for (i = 0; i < 15; i++)
            if (!tlist[i][0])
                break;
            else {
                uint16_t tmp_confidence;
                number_samples = (free_samples + busy_samples)/tlist[i][0];
                tmp_confidence = (200*tlist[i][1])
                                    / (number_samples*(number_samples+1));
                if (confidence < tmp_confidence && tlist[i][0] < 5000) {
                    pos = i;
                    confidence = tmp_confidence;                
                }            
            }
        
        if ((clusters_old[cluster_id].burst_size >= 2) && 
            (clusters_old[cluster_id].burst_size < 43))
                n_sources = ((confidence << 1) + 100) / 200;
        
        printf("Period:%d:cnt:%d:max confidence:%d:Beacon:%s\n", 
            tlist[pos][0], tlist[pos][1], 
            confidence, (n_sources)? "yes":"no");
   }
    
}

/*Check here for the interburst_serperation*/
void print_interarrival(uint16_t itr_cnt, int num_clusters) {
    uint16_t prev_burst = 0;   
    
    /* Label every burst according to the 
     * cluster to which it belongs
     */
    for (i = 0; i < num_bursts; i++)
    {
        sq_distance = 65535;
        for (j = 0; j < num_clusters; j++)
        {
            distance = ((clusters_old[j].burst_size - X[i][0]) * 
                        (clusters_old[j].burst_size - X[i][0])) +
                        ((clusters_old[j].plevel - X[i][1]) * 
                        (clusters_old[j].plevel - X[i][1]));
                    
            if ((j == 0) || (distance < sq_distance))
            {
                sq_distance = distance;
                burst_label[i] = j;
            }
        }
    }
    

    for (j = 0; j < num_clusters; j++)
    {
        // profiling_clusters_runs++;

        mean_period = 0;
        num_occurrences = 0;  max_cnt = 0; intf_idx = 0;
        // printf("K(%d,%d): ",clusters_old[j].burst_size, clusters_old[j].plevel);

        if(0)//for (i = 0; i < num_bursts; i++)
        {
            
            if (burst_label[i] == j)
            {
                // profiling_inter_bursts_runs++;
                if (num_occurrences) {
                    if (num_occurrences > 1)                                          
                        printf(",%lu", 
                               (uint32_t) (burst_ts[i] - burst_ts[prev_burst]));
                    else
                        printf("%lu", 
                               (uint32_t) (burst_ts[i] - burst_ts[prev_burst]));
                }
                prev_burst = i;
                num_occurrences++;
                
            }
            // Can also be here it should run.
        }
        // printf("\n");
        check_periodic(itr_cnt, j);
    }
    profiling_clusters_tot++;
    profiling_clusters_mean = profiling_clusters_runs/profiling_clusters_tot;

    profiling_inter_bursts_tot++;
    profiling_inter_bursts_mean = profiling_inter_bursts_runs/profiling_inter_bursts_tot;
    
    printf("PROFILING: total burst size: %d, total time function run %d, burst size mean: %d \n", 
            profiling_inter_bursts_runs,profiling_inter_bursts_tot,profiling_inter_bursts_mean);
    
    printf("PROFILING: total power level: %d, total times function run: %d, power level mean: %d \n",
            profiling_clusters_runs,profiling_clusters_tot,profiling_clusters_mean);


    // printf("PROFILING: total bursts created: %d, total time function run %d, burst mean: %d \n", 
    //         profiling_inter_bursts_runs,profiling_inter_bursts_tot,profiling_inter_bursts_mean);
    
    // printf("PROFILING: clusters created: %d, total times function run: %d, cluster mean: %d \n",
    //         profiling_clusters_runs,profiling_clusters_tot,profiling_clusters_mean);
}

/*-------------- END This is for classification --------------*/





int kmeans_old(struct record *record, int rle_ptr) //What is the rle_ptr when it is send in. 
{
    printf("rle_ptr: %d \n", rle_ptr);

          /* Initialize all values */
      cost = 0;
      cluster_test = 0;
      cluster_pref = -1;
      relative_ts = 0;
      record_ptr = 0; bsize = 0; plevel = 0; j = 0, num_bursts = 0;

      cost_final = 65535; 
      prev_cost_final = 65535;
      diffcost_between_clusters = 65535;
      cost_runs = 65535;
      iter = 1; 
      num_clusters_final = 0;
      prev_num_clusters_final = 0;
      nruns = 0;
      old_cost = 65535; cost_diff = 0;

      
     for (i = 0; i < 10; i++)
     {
        prev_K_final[i][0] = 0;
        prev_K_final[i][1] = 0;
        K_final[i][0] = 0;
        K_final[i][1] = 0;
     }

    // record->rssi_rle[250][0] = -1;
   
      free_samples = 0; busy_samples = 0;
      if (rle_ptr == RUN_LENGTH) rle_ptr--;
      /* Populate feature vectors for the clustering process */
      while(record_ptr < rle_ptr)
      {
        /* Process burst size and power level for a 
        * continuous stretch of RSSI readings 
        * above -90 dBm ( > power level 1)
        */

       /*Will create more burst if 2D first value is less than 1. */
        if (record->rssi_rle[record_ptr][0] <= POWER_LEVELS) { /*This runs 500 times*/                    
            
            while ((record->rssi_rle[record_ptr][0] > 1) &&
                    (record_ptr < rle_ptr))
            {
                // printf("bsize: %d \n", record->rssi_rle[record_ptr][1]);
                bsize += record->rssi_rle[record_ptr][1];                               //
                busy_samples += record->rssi_rle[record_ptr][1];
                plevel += record->rssi_rle[record_ptr][0] * record->rssi_rle[record_ptr][1];
                relative_ts += record->rssi_rle[record_ptr][1];
                record_ptr++;
                //
            }
            
            if (bsize > 0)
            {
                /* Populate feature matrix 
                * 0 - bsize, 
                * 1 - power level normalized by 16
                */
                X[num_bursts][0] = bsize;
                X[num_bursts][1] = (plevel << 4) / bsize;
                burst_ts[num_bursts] = relative_ts - bsize;
                num_bursts++;
                bsize = 0; 
                plevel = 0; 
            }
            /* We now see a burst of RSSI values 
            * below -90 dBm (power level 1). 
            * Update timestamps and free samples
            */
            if (record_ptr < rle_ptr) {
                relative_ts += record->rssi_rle[record_ptr][1];
                free_samples += record->rssi_rle[record_ptr][1];
                record_ptr++;
            }
        } else
            record_ptr++;
      }
    
    int d_cnt= 0;
    for(int i =0; i < 500; i++)
    {
        if(record->rssi_rle[i][0] == 1)
        {
            d_cnt +=1;
        }        
    }

    printf("num_bursts: %d, d_cnt %d \n", num_bursts, d_cnt);
    cluster_test = 0;
    while ((cluster_test < 10) && (diffcost_between_clusters > 3))
    {
        cost_runs = 65535;
        nruns = 0;
        cluster_test++;
        if (cluster_test > 1)	  
        {
            for (i = 0; i < num_clusters_final; i++)
            {
                prev_num_clusters_final = num_clusters_final;
                prev_K_final[i][0] = K_final[i][0];
                prev_K_final[i][1] = K_final[i][1];
                prev_cost_final = cost_final;
            }
        }

        while (nruns < 5)
        {
            old_cost = 65535; cost_diff = 0;	   
            iter = 1;
            
            if (old_cost > cost)
                cost_diff = old_cost - cost;
            else
                cost_diff = cost - old_cost;

            /* Initialize clusters
            */
            for (i = 0; i < 10; i++)
            {
                K[i][0] = 0;
                K[i][1] = 0;
                clusters_old[i].burst_size = 0;
                clusters_old[i].plevel = 0;
                clusters_old[i].mean_period = 0;
                clusters_old[i].num_occurrences = 0;
                clusters_old[i].intf_type = UNKNOWN;
            }
            /* We randomly initialize clusters to points in X */
            if (cluster_pref >= 0)
            {		
                K[0][0] = X[cluster_pref][0];
                K[0][1] = X[cluster_pref][1];
                for (i = 1; i < cluster_test; i++)
                {
                    idx = random_rand() % num_bursts;
                    K[i][0] = X[idx][0];
                    K[i][1] = X[idx][1];
                }
            }
            else
            {
                for (i = 0; i < cluster_test; i++)
                {
                    idx = random_rand() % num_bursts;
                    K[i][0] = X[idx][0];
                    K[i][1] = X[idx][1];
                }
            }

            /* Repeat until desired number of iterations 
            * OR the cost_diff is smaller than a 
            * randomly chosen value
            */
            while ((iter < 10) && (cost_diff > 3))
            {
            
                if (iter > 1)
                    old_cost = cost;
                
                /* Move centroid to the average 
                * of all related feature points
                */
                cost = 0;
                for (i = 0; i < num_bursts; i++)
                {
                    sq_distance = 65535;
                    for (j = 0; j < cluster_test; j++)
                    {
                        distance = ((K[j][0] - X[i][0])*(K[j][0] - X[i][0])) +
                            ((K[j][1] - X[i][1])*(K[j][1] - X[i][1]));
                                
                        if ((j == 0) || (distance < sq_distance)){
                            sq_distance = distance;
                            burst_label[i] = j;
                        }
                    }
                    cost = cost + sq_distance;
                }
                
                for (j = 0; j < cluster_test; j++)
                {
                    sum_points1 = 0; sum_points2 = 0; num_points = 0;
                    for (i = 0; i < num_bursts; i++)
                    {
                        if (burst_label[i] == j)
                        {
                            sum_points1 = sum_points1 + X[i][0];
                            sum_points2 = sum_points2 + X[i][1];
                            num_points++;
                        }
                    }
                
                    if (num_points)
                    {
                        K[j][0] = sum_points1 / num_points;
                        K[j][1] = sum_points2 / num_points;
                    }
                    else{
                        K[j][0] = 0x7FFF;
                        K[j][1] = 0x7FFF;
                    }
                }
                
                cost /= num_bursts;

                /* Added an enhancement to the clustering algorithm. 
                * Here, instead of randomly selecting cluster points in 
                * the next iteration, we explicitly choose burst features 
                * that have a large cost, i.e. they have a large distance 
                * from their cluster head in the current iteration.
                * This should hopefully solve the problem of outlier features 
                * not being classified separately.
                */
                min_distance = 0;
                for (i = 0; i < num_bursts; i++)
                {	
                    sq_distance = ( (K[burst_label[i]][0] - X[i][0]) * 
                                    (K[burst_label[i]][0] - X[i][0])) +
                                    ((K[burst_label[i]][1] - X[i][1]) * 
                                    (K[burst_label[i]][1] - X[i][1]));
                    
                    if (sq_distance > min_distance) {
                        min_distance = sq_distance;
                        cluster_pref = i;
                    }
                }
                
                if (old_cost > cost)
                    cost_diff = old_cost - cost;
                else
                    cost_diff = cost - old_cost;
                
                
                iter++;
            }
            
            if (cost_runs > cost) 
            {
                cost_runs = cost;
                num_clusters_final = 0;
                for (i = 0; i < cluster_test; i++)
                {
                    if (K[i][0] != 0x7FFF && K[i][1] != 0x7FFF) {		      
                    K_final[num_clusters_final][0] = K[i][0];
                    K_final[num_clusters_final][1] = K[i][1];
                    num_clusters_final++;
                    }
                }
            }
            nruns++;
        } /* (nruns < 5) */
        
        if (cost_final > cost_runs)
        {
            diffcost_between_clusters = cost_final - cost_runs;
            cost_final = cost_runs;
        }
        else
        {
            diffcost_between_clusters = cost_runs - cost_final;
            cost_final = cost_runs;
        }
    } /* ((cluster_test < 10) && (diffcost_between_clusters > 3)) */
    
    printf("num_clusters:%d, cost:%ld, samples: %lu (%lu,%lu)\n", 
            prev_num_clusters_final, prev_cost_final, 
            free_samples + busy_samples, 
            free_samples, busy_samples);
    for (i = 0; i < prev_num_clusters_final; i++)
    {	  
	  clusters_old[i].burst_size = prev_K_final[i][0];
	  clusters_old[i].plevel = prev_K_final[i][1];	  
	  clusters_old[i].mean_period = 0;
	  clusters_old[i].num_occurrences = 0;
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
