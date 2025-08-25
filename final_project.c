#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

//for the background
#define SCREEN_WIDTH 319
#define SCREEN_HEIGHT 239

// Sample 16x16 pixel array - a simple smiley face
#define ARRAY_WIDTH 319
#define ARRAY_HEIGHT 239
	
#define BLACK 0x0000
#define WHITE 0xFFFF
#define YELLOW 0xFFE0
#define RED 0xF800
	

// Define constants
#define N 4 // Number of boxes
#define BOX_SIZE 40 // Size of each box
#define SPACING 7   // Space between boxes
#define DELAY 1000  // Delay between box displays (1 second)
#define PATTERN_LENGTH 4 // Number of boxes to display before stopping

// Key setup
#define KEY_BASE 0xFF200050  // Base address of pushbutton keys

// Function prototypes for interrupt handling
static void handler(void) __attribute__ ((interrupt ("machine")));
void set_keys(void);
void disable_key_interrupts(void);
void enable_key_interrupts(void);
void keys_ISR(void);

// Location of each box
int x_box[N];
int y_box[N];

// Color of each box
short int color_box[N];

// Declare all functions
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void wait_for_vsync();
void draw_box(int i, short int color);
void delay(int milliseconds);
void draw_character(int x, int y, char ch, short int color);
void draw_text(int x, int y, const char *text, short int color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void rectangle_starting_positions();
void draw(int box_index);

// Global variables
volatile int pixel_buffer_start; // Global variable
short int Buffer1[240][512];    // Front buffer
short int Buffer2[240][512];    // Back buffer

int score = 0;  // Initialize score
int wait = 0;
int wrong = 0;
char score_text[10];
char round_text[10];  // Reduced size from 100 to 10
int level = 1;

// Variables for interrupt handling
volatile int key_pressed = -1;   // -1 means no key pressed
volatile int * pixel_ctrl_ptr = 0;
volatile int * KEY_ptr = 0;
int pattern[PATTERN_LENGTH];     // Store the pattern
int current_index = 0;           // Track which pattern index we're on


int main(void) {
	int DELAYS = 1000;
	
	
    pixel_ctrl_ptr = (int *)0xFF203020;

    /* Set front pixel buffer to Buffer 1 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer1; // First store the address in the back buffer
    /* Now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    pixel_buffer_start = *pixel_ctrl_ptr; // Initialize pointer

    // Initialize KEY pointer
    KEY_ptr = (int *)KEY_BASE;

    //random number generator
    srand(time(NULL));

    // Initialize location and colors of rectangles
    rectangle_starting_positions();

    // Set up interrupt handling
    int mstatus_value, mtvec_value, mie_value;
    
    mstatus_value = 0b1000;      // Interrupt bit mask
    // Disable interrupts
    __asm__ volatile ("csrc mstatus, %0" :: "r"(mstatus_value));
    
    mtvec_value = (int) &handler;  // Set trap address
    __asm__ volatile ("csrw mtvec, %0" :: "r"(mtvec_value));
    
    // Disable all interrupts
    __asm__ volatile ("csrr %0, mie" : "=r"(mie_value));
    __asm__ volatile ("csrc mie, %0" :: "r"(mie_value));
    
    //KEY interrupts
    set_keys();
    
    //interrupts globally
    __asm__ volatile ("csrs mstatus, %0" :: "r"(mstatus_value));

    clear_screen(); // Clear the screen

    /* Set back pixel buffer to Buffer 2 */
    *(pixel_ctrl_ptr + 1) = (int) &Buffer2;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // We draw on the back buffer
    clear_screen(); // Clear the back buffer

    while (1) {

clear_screen();
        
        int box_count = 0;
        
        // Update score and level text
        sprintf(score_text, "%d", score);
        sprintf(round_text, "%d", level);
        
        // Adjust difficulty based on score
        if((score % 5 == 0) && (score != 0)) {
			level += 1;
            DELAYS -= 200;
        }
        
        // Generate a new random pattern
        for (int i = 0; i < PATTERN_LENGTH; i++) {
            pattern[i] = rand() % N; // Random box between 0 and 3
        }
        
        // Display the pattern
        while (box_count < PATTERN_LENGTH) {
		clear_screen();
			//count down before game begins
			if(box_count == 0){
			delay(2000);

			draw_text(150, 100, "1", 0xFFFF); // White text
			wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                        clear_screen();

	delay(2000);
            clear_screen();
			draw_text(150, 100, "2", 0xFFFF); // White text
			wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
			delay(2000);
            clear_screen();
			draw_text(150, 100, "3", 0xFFFF); // White text
			wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
			delay(2000);
            clear_screen();
			wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
			}
			
            delay(DELAYS);
            clear_screen();
            draw_text(80, 50, "SIMON SAYS", 0xFFFF); // White text
            draw_text(10, 10, score_text, 0xFFFF); // White text
            draw_text(300, 10, round_text, 0xFFFF); // White text
            wait_for_vsync();
            pixel_buffer_start = *(pixel_ctrl_ptr + 1);
            delay(DELAYS);

            // Draw the next box in the pattern
            draw(pattern[box_count]);
            box_count++;
            draw_text(80, 50, "SIMON SAYS", 0xFFFF); // White text
            draw_text(10, 10, score_text, 0xFFFF); // White text
            draw_text(300, 10, round_text, 0xFFFF); // White text

            wait_for_vsync(); // Swap front and back buffers
            pixel_buffer_start = *(pixel_ctrl_ptr + 1); // New back buffer
			
        }

        // After displaying boxes, clear the screen 
        delay(DELAYS);
        clear_screen();
        draw_text(80, 50, "SIMON SAYS", 0xFFFF); // White text
        draw_text(10, 10, score_text, 0xFFFF); // White text
        draw_text(300, 10, round_text, 0xFFFF); // White text
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        wrong = 0;
        current_index = 0;  // Reset current pattern index

        // Show all boxes in default state while waiting for input
        clear_screen();
        for (int j = 0; j < N; j++) {
            draw_box(j, color_box[j] & 0x7BEF); // Draw dimmed version of all boxes
        }
        draw_text(80, 50, "SIMON SAYS", 0xFFFF); // White text
        draw_text(10, 10, score_text, 0xFFFF); // White text
        draw_text(300, 10, round_text, 0xFFFF); // White text
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);

        // Now enable KEY interrupts and wait for player input
        enable_key_interrupts();
        
        // Wait for player to complete pattern or make a mistake
        while (current_index < PATTERN_LENGTH && wrong == 0) {
            // Wait for key_pressed to be updated by interrupt handler
            if (key_pressed != -1) {
                // Show the pressed box
                clear_screen();
                for (int j = 0; j < N; j++) {
                    if (j == key_pressed) {
                        draw_box(j, color_box[j]); // Full brightness for pressed box
                    } else {
                        draw_box(j, color_box[j] & 0x7BEF); // Dimmed for others
                    }
                }
                draw_text(80, 50, "SIMON SAYS", 0xFFFF); // White text
                draw_text(10, 10, score_text, 0xFFFF); // White text
                draw_text(300, 10, round_text, 0xFFFF); // White text
                wait_for_vsync();
                pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                
                // Check if correct key was pressed
                if (key_pressed == pattern[current_index]) {
                    current_index++; // Move to next pattern element
                } else {
                    wrong = 1;
                    score = 0;
                    level = 1;
                }
                
                // Reset key_pressed for next input
                key_pressed = -1;
                
                // Add a delay
                delay(300);
                
                // Redraw boxes in dimmed state if there are more inputs needed
                if (current_index < PATTERN_LENGTH && wrong == 0) {
                    clear_screen();
                    for (int j = 0; j < N; j++) {
                        draw_box(j, color_box[j] & 0x7BEF); // Draw dimmed version of all boxes
                    }
                    draw_text(80, 50, "SIMON SAYS", 0xFFFF); // White text
                    draw_text(10, 10, score_text, 0xFFFF); // White text
                    draw_text(300, 10, round_text, 0xFFFF); // White text
                    wait_for_vsync();
                    pixel_buffer_start = *(pixel_ctrl_ptr + 1);
                }
            }
        }
        
        // Disable key interrupts until next round
        disable_key_interrupts();

        // Clear screen before starting the next round
        clear_screen();
        
        if (wrong == 0) {
            score += 1;
        } else {
           score = 0;
			level = 1;
		
        }
        
        wait_for_vsync();
        pixel_buffer_start = *(pixel_ctrl_ptr + 1);
        delay(2000); // Show result for 2 seconds
    }
}

/**********************************************************************
INNTERUPT HANDLER
 **********************************************************************/
void handler(void) {
    int mcause_value;
    __asm__ volatile ("csrr %0, mcause" : "=r"(mcause_value));
    
    if (mcause_value == 0x80000012) // KEY port (IRQ18)
        keys_ISR();
}

/**********************************************************************
 * Configure KEY port
 **********************************************************************/
void set_keys(void) {
    volatile int *KEY_ptr = (int *)KEY_BASE;
    
    // Clear EdgeCapture register
    *(KEY_ptr + 3) = 0xF;  
    
    // Configure interrupt but dont enable yet
    *(KEY_ptr + 2) = 0xF;  // Set all bits in interrupt mask
}

/**********************************************************************
 * Enable KEY interrupts in mie register
 **********************************************************************/
void enable_key_interrupts(void) {
    int mie_value = 0x40000;  // KEY port (IRQ18)
    __asm__ volatile ("csrs mie, %0" :: "r"(mie_value));
}

/**********************************************************************
 * Disable KEY interrupts in mie register
 **********************************************************************/
void disable_key_interrupts(void) {
    int mie_value = 0x40000;  // KEY port (IRQ18)
    __asm__ volatile ("csrc mie, %0" :: "r"(mie_value));
}

/**********************************************************************
 * KEY port interrupt service routine
 **********************************************************************/
void keys_ISR(void) {
    volatile int *KEY_ptr = (int *)KEY_BASE;
    int pressed;
    
    // Read which KEY was pressed from EdgeCapture register
    pressed = *(KEY_ptr + 3);  // Read EdgeCapture (offset 0xC)
    
    // Clear EdgeCapture register
    *(KEY_ptr + 3) = pressed;  // Write back to clear
    
    // Determine which box was pressed
    if (pressed & 0x1) key_pressed = 3;           
    else if (pressed & 0x2) key_pressed = 2;
    else if (pressed & 0x4) key_pressed = 1;
    else if (pressed & 0x8) key_pressed = 0;
}

/*
plot_pixel: This function plots a pixel at the specified (x, y) coordinates with the given color.
address = pixel_buffer_start + (y << 10) + (x << 1);: Calculates the address of the pixel in the buffer.
*address = line_color;: Sets the pixel color.
*/
void plot_pixel(int x, int y, short int line_color) {
    volatile short int *address;
    address = (volatile short int *)(pixel_buffer_start + (y << 10) + (x << 1));
    *address = line_color;
}

void wait_for_vsync() {
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020; // Base address
    int status;
    *pixel_ctrl_ptr = 1; // Starts sync process. Must write a 1 into the first reg (front buffer) to allow swaps
    status = *(pixel_ctrl_ptr + 3); // Read status register    

    while ((status & 0x01) != 0) { // Polling loop waits for S bit to go to 0 (indicates swap happened)
        status = *(pixel_ctrl_ptr + 3);
    }
}

// This function is from class, specifically lecture 20. Clears the screen.
void clear_screen() {
    for (int x = 0; x < 320; x++) {
        for (int y = 0; y < 240; y++)
            plot_pixel(x, y, 0x0000); // 0x0000 is the color for black
    }
}

void rectangle_starting_positions() {
    // Initialize the positions and colors of the 4 boxes
    int box_size = BOX_SIZE; // Size of each box
    int spacing = SPACING;   // Space between boxes

    // Box 1: Green
    x_box[0] = 70;
    y_box[0] = 150;
    color_box[0] = 0x07E0; // Green

    // Box 2: Red
    x_box[1] = x_box[0] + box_size + spacing;
    y_box[1] = 150;
    color_box[1] = 0xF800; // Red

    // Box 3: Blue
    x_box[2] = x_box[1] + box_size + spacing;
    y_box[2] = 150;
    color_box[2] = 0x001F; // Blue

    // Box 4: Yellow
    x_box[3] = x_box[2] + box_size + spacing;
    y_box[3] = 150;
    color_box[3] = 0xFFE0; // Yellow
}

void draw(int box_index) {
    // Draw only the selected box in its designated color
    for (int i = 0; i < N; i++) {
        if (i == box_index) {
            // Draw the selected box in its color
            draw_box(i, color_box[i]);
        } else {
            // Draw all other boxes in black
            draw_box(i, 0x0000); // 0x0000 is black
        }
    }
}

void draw_box(int i, short int color) {
    int box_size = BOX_SIZE; // Size of each box
    for (int y = y_box[i]; y < y_box[i] + box_size; y++) {
        for (int x = x_box[i]; x < x_box[i] + box_size; x++) {
            plot_pixel(x, y, color);
        }
    }
}

// Function to introduce a delay
void delay(int milliseconds) {
    volatile int i;
    for (i = 0; i < milliseconds * 1000; i++); // Simple busy-wait loop
}

// Function to draw a character
void draw_character(int x, int y, char ch, short int color) {
    switch (ch) {
	case '0':
    draw_line(x, y, x + 15, y, color);
    draw_line(x, y, x, y + 20, color);
    draw_line(x, y + 20, x + 15, y + 20, color);
    draw_line(x + 15, y, x + 15, y + 20, color);
    break;
	   case '1':
            draw_line(x + 7, y, x + 7, y + 20, color);
            draw_line(x + 3, y + 5, x + 7, y, color);
            break;
        case '2':
            draw_line(x, y, x + 15, y, color);
            draw_line(x + 15, y, x + 15, y + 10, color);
            draw_line(x, y + 10, x + 15, y + 10, color);
            draw_line(x, y + 10, x, y + 20, color);
            draw_line(x, y + 20, x + 15, y + 20, color);
            break;
        case '3':
            draw_line(x, y, x + 15, y, color);
            draw_line(x + 15, y, x + 15, y + 20, color);
            draw_line(x, y + 10, x + 15, y + 10, color);
            draw_line(x, y + 20, x + 15, y + 20, color);
            break;
        case '4':
            draw_line(x, y, x, y + 10, color);
            draw_line(x, y + 10, x + 15, y + 10, color);
            draw_line(x + 15, y, x + 15, y + 20, color);
            break;
        case '5':
            draw_line(x + 15, y, x, y, color);
            draw_line(x, y, x, y + 10, color);
            draw_line(x, y + 10, x + 15, y + 10, color);
            draw_line(x + 15, y + 10, x + 15, y + 20, color);
            draw_line(x, y + 20, x + 15, y + 20, color);
            break;
    case '6':
    draw_line(x, y, x + 15, y, color);         
    draw_line(x, y, x, y + 20, color);         
    draw_line(x, y + 10, x + 15, y + 10, color); 
    draw_line(x, y + 20, x + 15, y + 20, color); 
    draw_line(x + 15, y + 10, x + 15, y + 20, color); 
    break;
        case '7':
            draw_line(x, y, x + 15, y, color);
            draw_line(x + 15, y, x + 7, y + 20, color);
            break;
        case '8':
            draw_line(x, y, x + 15, y, color);
            draw_line(x, y, x, y + 20, color);
            draw_line(x, y + 10, x + 15, y + 10, color);
            draw_line(x, y + 20, x + 15, y + 20, color);
            draw_line(x + 15, y, x + 15, y + 20, color);
            break;
        case '9':
            draw_line(x, y, x + 15, y, color);
            draw_line(x, y, x, y + 10, color);
            draw_line(x, y + 10, x + 15, y + 10, color);
            draw_line(x + 15, y, x + 15, y + 20, color);
            break;
        case 'S':
            draw_line(x, y, x + 15, y, color);  // Top horizontal
            draw_line(x, y, x, y + 10, color);  // Left vertical
            draw_line(x, y + 10, x + 15, y + 10, color);  // Middle horizontal
            draw_line(x + 15, y + 10, x + 15, y + 20, color);  // Right vertical
            draw_line(x, y + 20, x + 15, y + 20, color);  // Bottom horizontal
            break;
        case 'I':
            draw_line(x + 7, y, x + 7, y + 20, color);  // Vertical
            break;
        case 'M':
            draw_line(x, y + 20, x, y, color);  // Left vertical
            draw_line(x, y, x + 7, y + 10, color);  // Diagonal down
            draw_line(x + 7, y + 10, x + 15, y, color);  // Diagonal up
            draw_line(x + 15, y, x + 15, y + 20, color);  // Right vertical
            break;
        case 'O':
            draw_line(x, y, x + 15, y, color);  // Top horizontal
            draw_line(x, y, x, y + 20, color);  // Left vertical
            draw_line(x, y + 20, x + 15, y + 20, color);  // Bottom horizontal
            draw_line(x + 15, y, x + 15, y + 20, color);  // Right vertical
            break;
        case 'N':
            draw_line(x, y + 20, x, y, color);  // Left vertical
            draw_line(x, y, x + 15, y + 20, color);  // Diagonal
            draw_line(x + 15, y + 20, x + 15, y, color);  // Right vertical
            break;
        case 'A':
            draw_line(x, y + 20, x + 7, y, color);  // Diagonal up
            draw_line(x + 7, y, x + 15, y + 20, color);  // Diagonal down
            draw_line(x + 3, y + 10, x + 12, y + 10, color);  // Middle horizontal
            break;
        case 'Y':
            draw_line(x, y, x + 7, y + 10, color);  // Diagonal down
            draw_line(x + 15, y, x + 7, y + 10, color);  // Diagonal down
            draw_line(x + 7, y + 10, x + 7, y + 20, color);  // Vertical
            break;
        case ' ':
            // Space character (do nothing)
            break;
    }
}

// Function to draw text
void draw_text(int x, int y, const char *text, short int color) {
    while (*text) {
        draw_character(x, y, *text, color); // Draw the current character
        x += 18; // Move to the next character position (18 pixels apart for bigger font)
        text++; // Move to the next character in the string
    }
}

// Function to draw a line
void draw_line(int x0, int y0, int x1, int y1, short int line_color) {
    bool is_steep = abs(y1 - y0) > abs(x1 - x0);
    if (is_steep) {
        int temp0 = x0, temp1 = x1;
        x0 = y0; y0 = temp0;
        x1 = y1; y1 = temp1;
    }
    if (x0 > x1) {
        int tempX = x0, tempY = y0;
        x0 = x1; x1 = tempX;
        y0 = y1; y1 = tempY;
    }

    int deltax = x1 - x0;
    int deltay = abs(y1 - y0);
    int error = -(deltax / 2);
    int y = y0;
    int y_step = (y0 < y1) ? 1 : -1;

    for (int x = x0; x <= x1; x++) {
        if (is_steep) {
            plot_pixel(y, x, line_color);
        } else {
            plot_pixel(x, y, line_color);
        }
        error += deltay;
        if (error > 0) {
            y += y_step;
            error -= deltax;
        }
    }
}



