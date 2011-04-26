/*
 * graphics_lcd.h
 *
 *  Created on: Apr 23, 2011
 *      Author: maulik
 */

#ifndef GRAPHICS_LCD_H_
#define GRAPHICS_LCD_H_

#include "hal_defs.h"
#include "hal_types.h"

void draw_drums(void);
void small_delay(int x);
void draw_piano(void);
void clear_screen_lcd(void);
void pixel(char S_R, char x, char y);
void line(char S_R, char x1, char y1, char x2, char y2);
void circle(char S_R, int x, int y, int r);
void set_text_position(char x, char y);


#endif /* GRAPHICS_LCD_H_ */
