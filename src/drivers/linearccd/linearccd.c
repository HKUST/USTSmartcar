/*******************************************

linearccd.c

Authored by Louis Mo
for HKUST SmartCar team 2013

Edited by John Ching

*******************************************/

#include  "include.h"
#include  "linearccd.h"

/*********** CCD related counter ************/
u16 g_u16_ccd_sample_clock=0;
u16 g_u16_ccd_long_SI_counter=800;

/*********** CCD related status flag ************/
int g_int_SI_state_flag=0;                    // SI flag
int g_int_sampling_state_flag=0;              // sampling state flag
int g_int_trash_sample_flag=0;                // notice dummy sample
int g_int_ccd_operation_state=0;

/*********** CCD related sample result & array ************/
char g_char_ar_ccd_pixel[256];                // 1-line pixel array
char g_char_ar_ccd_previous_pixel[256];       // previous pixel array
char g_char_ar_ccd_benchmark_one[256];        // benchmark 1
char g_char_ar_ccd_benchmark_two[256];        // benchmark 2
char g_char_ar_ccd_benchmark_reuse[256];      // reuseable benchmark

/*********** CCD track decision related variable ************/
u16 g_u16_ccd_left_pos=0;                // dynamic left edge scan
u16 g_u16_ccd_right_pos=0;               // dynamic right edge scan
u16 g_u16_ccd_previous_left_pos=0;
u16 g_u16_ccd_previous_right_pos=0;
//u16 g_u16_ccd_valid_range=0;                  // dynamic valid range

/*********** CCD basic library ************/
void ccd_sampling(char array[], int state){  
  g_int_ccd_operation_state = state;  
  while(g_int_ccd_operation_state == 1){        
       ccd_clock_turn();       
       if(g_u16_ccd_long_SI_counter == 800){   
        ccd_trigger_SI();                          
       }       
       // When PIT0 clock = 2.5us 
       //  1ms/2.5us = 500
       //2.5ms/2.5us = 1000 (Nice response in lab testing)     
       ccd_detect_track(array);       
       ccd_SI_failing_edge_condition();       
       ccd_finish_one_sampling(array);       
       g_u16_ccd_long_SI_counter++;       
       g_u16_ccd_sample_clock++;
  }   
}

void ccd_clock_turn(){
    gpio_turn(PORTB, 9); // Gen 2 main board Clock
}

void ccd_trigger_SI(){
    if(g_int_SI_state_flag == 0 ){
               g_int_SI_state_flag = 1;              // SI Flag on
               g_int_sampling_state_flag = 1;        // sampling Flag on
               g_u16_ccd_sample_clock = 0;
               gpio_set(PORTB, 8, 1);                // Gen 2 main board SI rising edge
    }
}

void ccd_detect_track(char array[]){
   if(gpio_get(PORTB, 10) == 1) {  
   // if CCD receives black (2nd gen)
        array[g_u16_ccd_sample_clock] = '|';
   } else {                        
   // if CCD receives white (2nd gen)
        array[g_u16_ccd_sample_clock] = '_';
   }
}

void ccd_SI_failing_edge_condition(){
  if(g_u16_ccd_sample_clock == 1 && g_int_SI_state_flag == 1){ // condition for Longer SI failing edge to end
        gpio_set(PORTB, 8, 0); // Gen 2 SI faling edge
        g_u16_ccd_long_SI_counter = 0;
  }
}

void ccd_finish_one_sampling(char array[]){  
     if(g_u16_ccd_sample_clock == 512){
          g_int_SI_state_flag = 0;          // SI Flag off
          g_int_sampling_state_flag = 0;    // Sampling flag off
        
     ccd_shift_sample_to_manageable_position(array);
     ccd_scan_dummy_sample_result(array);     
          
          if(g_int_trash_sample_flag == 1){
             // do nth
          }else{
          ccd_output_sample_to_UART(array);
          g_int_ccd_operation_state = 0;
          }
     }       
}

void ccd_output_sample_to_UART(char array[]){
     uart_sendStr(UART3,"\n"); 
     uart_sendStr(UART3,"The Sample is:");
     ccd_print(array);
     uart_sendStr(UART3,"\n"); 
    
     ccd_output_edge_to_UART(); //temp
}

void ccd_shift_sample_to_manageable_position(char array[]){
      u16 i; 
      for( i = 0 ; i < 256 ; i++){
      array[i] = array[i+2];
      }
}

void ccd_scan_dummy_sample_result(char array[]){
      u16 i; 
      u16 dummy = 0;
      g_int_trash_sample_flag = 0; // reset trash flag
      
      for( i = 0 ; i < 256 ; i++){
        if(array[i] == '_'){
          dummy++; }
      }
      
      if(dummy == 256){              // if dummy sample detect
        g_int_trash_sample_flag = 1; // trash flag ON
      }
}

void ccd_print(char array[]){
      u16 i;  
      for( i = 0 ; i < 256 ; i++){
        uart_putchar(UART3,array[i]); // print sample to UART
      }
}

/*********** CCD track decision ***********
void ccd_analyze_track_from_sample(char array[]){  
  
  u16 i;  
  u16 straight_line_similarity=0;
  for(i = 0 ; i < g_u16_ccd_left_edge_pos ; i++){ // scan 1st half left array
    if(array[i] == '|'){
    straight_line_similarity++;
    }
  }
  
  for(i = g_u16_ccd_right_edge_pos; i < 256 ; i++){
    if(array[i] == '|'){
    straight_line_similarity++;
    }
  }   
  
  if( (g_u16_ccd_valid_range - straight_line_similarity) <= 10 ){ // check difference
  uart_sendStr(UART3,"*.* Straight Line *.*");  
  // straight line case
  }
  
  // ............... to be edit
  
}*/

void ccd_save_previous_sampling(char input_array[], char output_stored_array[]){ 
      u16 i;  
      for( i = 0 ; i < 256 ; i++){
        output_stored_array[i] = input_array[i]; // copy previous array, before next sampling
      }
}

void ccd_decide_range_for_detection(char array[]){
      u16 i;  
      u16 update_left;
      u16 update_right;
      
      for( i = 0 ; i < 128 ; i++){ // scan 1st half left array
        if(array[i] == '|'){
        g_u16_ccd_left_pos = i; // set left edge
        }
      }
      
      for( i = 256 ; i > 128 ; i--){ // scan 2nd half right array
        if(array[i] == '|'){
        g_u16_ccd_right_pos = i; // set right edge 
        }
      }
      
      //g_u16_ccd_valid_range = g_u16_ccd_left_edge_pos + (256 - g_u16_ccd_right_edge_pos);    

     /********Left *******/
     uart_sendStr(UART3,"This Left edge position is:"); 
     printf("%d", g_u16_ccd_left_pos);
     uart_sendStr(UART3,"\n");
     
     uart_sendStr(UART3,"Previous left edge position is:"); 
     printf("%d", g_u16_ccd_previous_left_pos);
     uart_sendStr(UART3,"\n");
     
     update_left = (g_u16_ccd_left_pos+g_u16_ccd_previous_left_pos)/2; // factor : 50% from previous average, 50% from current effect
     
     uart_sendStr(UART3,"Final left edge position for decision is:"); 
     printf("%d", update_left);
     uart_sendStr(UART3,"\n");
     uart_sendStr(UART3,"\n");

     g_u16_ccd_previous_left_pos = update_left;        //update previous pos to current pos 
     g_u16_ccd_left_pos = update_left;                 //final pos before this function end
     
     /********Right *******/
     uart_sendStr(UART3,"This Right edge position is:"); 
     printf("%d", g_u16_ccd_right_pos);
     uart_sendStr(UART3,"\n");
     
     uart_sendStr(UART3,"Previous right edge position is:"); 
     printf("%d", g_u16_ccd_previous_right_pos);
     uart_sendStr(UART3,"\n");
     
     update_right = (g_u16_ccd_right_pos+g_u16_ccd_previous_right_pos)/2; // factor : 50% from previous average, 50% from current effect
     
     uart_sendStr(UART3,"Final right edge position for decision is:"); 
     printf("%d", update_right);
     uart_sendStr(UART3,"\n");
     
     g_u16_ccd_previous_right_pos = update_left;         //update previous pos to current pos 
     g_u16_ccd_right_pos = update_right;                 //final pos before this function end
}

void ccd_output_edge_to_UART(){
     // temporary function
     uart_sendStr(UART3,"\n");
     uart_sendStr(UART3,"Left edge valid position is:");
     printf("%d", g_u16_ccd_left_pos);
     uart_sendStr(UART3,"\n");
     
     uart_sendStr(UART3,"Right edge valid position is:");
     printf("%d", g_u16_ccd_right_pos);
     uart_sendStr(UART3,"\n");
     uart_sendStr(UART3,"\n");
}

/*********** CCD sample filtering ************/
void ccd_sample_filtering(char array[]){
  
  // compare currect sample with previos sample
  
  int ccd_same_pixel_count = 0;
  char ccd_similar_count_digit[3];
  int x;
  int y;
  
  // eveulate the similarity between previous sample and currenct sample
  // by int value 0 to 127, higher is better
  for( x = 0 ; x < 128 ; x++){
          if ( g_char_ar_ccd_previous_pixel[x] == g_char_ar_ccd_pixel[x]){
          ccd_same_pixel_count++;
          }
  }
      
  convert_char_to_readable_integer(ccd_same_pixel_count,ccd_similar_count_digit);
   
  for( x = 2 ; x >= 0 ; x--){
  uart_putchar(UART3,ccd_similar_count_digit[x]);
  }
  
  uart_sendStr(UART3," number of current pixels are same as pervious sample");
  uart_sendStr(UART3,"\n");     
  
  // if the current sample is unsatisfactory, replace the previous sample
  if(ccd_same_pixel_count > 1 && ccd_same_pixel_count < 60){
    
     for( y = 0 ; y < 128 ; y++){
     g_char_ar_ccd_pixel[y] = g_char_ar_ccd_previous_pixel[y]; 
     } 
     
      uart_sendStr(UART3,"\t\t\t\t");    
      uart_sendStr(UART3,"**************************************************************************");   
      uart_sendStr(UART3,"\n");      
      
      uart_sendStr(UART3,"\t\t\t\t");    
      uart_sendStr(UART3,"** The current sample has been filtered and replaced by previous sample **");
      uart_sendStr(UART3,"\n");     
      
      uart_sendStr(UART3,"\t\t\t\t");    
      uart_sendStr(UART3,"**************************************************************************");   
      uart_sendStr(UART3,"\n");     
  }
  
  // instead of using previous sample, use the benchmark sample
  // ... to be edit
  
}


/*********** Supplementary function************/

void convert_char_to_readable_integer(int input_int, char output_char[]){  
  if(input_int >= 0 && input_int < 10){    
    output_char[0] = input_int + '0';             // ones 
    output_char[1] = '0';                         // tens
    output_char[2] = '0';                         // hundreds    
  } else if (input_int >= 10 && input_int < 100){    
    output_char[0] = (input_int%10) + '0';        // ones
    output_char[1] = (input_int/10)%10 + '0';     // tens
    output_char[2] = '0';                         // hundreds    
  } else if (input_int >= 100 && input_int < 1000){    
    output_char[0] = (input_int%10) + '0';        // ones
    output_char[1] = (input_int/10)%10 + '0';     // tens
    output_char[2] = (input_int/100)%10 + '0';    // hundreds
  }  
}
