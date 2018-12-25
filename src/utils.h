/*
 * utils.h
 */

#ifndef UTILS_H_
#define UTILS_H_

#define DEFAULT_LINE_LEN		60

#define ANSI_COLOR_RED     \x1b[31m
#define ANSI_COLOR_GREEN   \x1b[32m
#define ANSI_COLOR_YELLOW  \x1b[33m
#define ANSI_COLOR_BLUE    \x1b[34m
#define ANSI_COLOR_MAGENTA \x1b[35m
#define ANSI_COLOR_CYAN    \x1b[36m
#define ANSI_COLOR_RESET   \x1b[0m

size_t hex2bin(uint8_t *dst, const char *src);
void print_ln_len(char symbol, uint8_t cnt);
void print_ln(char symbol);
void print_hex(const uint8_t *data, uint32_t len, const char *delimiter);
void print_hex_ln(const uint8_t *data, uint32_t len, const char *delimiter);

#endif /* UTILS_H_ */
