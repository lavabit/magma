
/**
 * @file /magma/core/host/color.c
 *
 * @brief Functions for colorizing the console output.
 */

#include "magma.h"

bool_t color_supported(void) {

	const chr_t *term = getenv("TERM");
    const bool_t result =
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("xterm")) ||
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("xterm-color")) ||
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("xterm-256color")) ||
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("screen")) ||
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("screen-256color")) ||
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("linux")) ||
        !st_cmp_ci_eq(NULLER((chr_t *) term), NULLER("cygwin"));
#ifdef  MAGMA_ENGINE_CONFIG_GLOBAL_H
    return result && !magma.system.daemonize && !magma.output.file;
#else
    return result;
#endif
}

const chr_t * color_reset(void) {
	return color_supported() ? MAGMA_COLOR_RESET : "";
}

const chr_t * color_red(void) {
	return color_supported() ? MAGMA_COLOR_RED : "";
}

const chr_t * color_green(void) {
	return color_supported() ? MAGMA_COLOR_GREEN : "";
}

const chr_t * color_yellow(void) {
	return color_supported() ? MAGMA_COLOR_YELLOW : "";
}

const chr_t * color_blue(void) {
	return color_supported() ? MAGMA_COLOR_BLUE : "";
}

const chr_t * color_purple(void) {
	return color_supported() ? MAGMA_COLOR_PURPLE : "";
}

const chr_t * color_cyan(void) {
	return color_supported() ? MAGMA_COLOR_CYAN : "";
}

const chr_t * color_white(void) {
	return color_supported() ? MAGMA_COLOR_WHITE : "";
}

const chr_t * color_red_bold(void) {
	return color_supported() ? MAGMA_COLOR_RED_BOLD : "";
}

const chr_t * color_green_bold(void) {
	return color_supported() ? MAGMA_COLOR_GREEN_BOLD : "";
}

const chr_t * color_yellow_bold(void) {
	return color_supported() ? MAGMA_COLOR_YELLOW_BOLD : "";
}

const chr_t * color_blue_bold(void) {
	return color_supported() ? MAGMA_COLOR_BLUE_BOLD : "";
}

const chr_t * color_purple_bold(void) {
	return color_supported() ? MAGMA_COLOR_PURPLE_BOLD : "";
}

const chr_t * color_cyan_bold(void) {
	return color_supported() ? MAGMA_COLOR_CYAN_BOLD : "";
}

const chr_t * color_white_bold(void) {
	return color_supported() ? MAGMA_COLOR_WHITE_BOLD : "";
}

const chr_t * color_red_underline(void) {
	return color_supported() ? MAGMA_COLOR_RED_UNDERLINE : "";
}

const chr_t * color_green_underline(void) {
	return color_supported() ? MAGMA_COLOR_GREEN_UNDERLINE : "";
}

const chr_t * color_yellow_underline(void) {
	return color_supported() ? MAGMA_COLOR_YELLOW_UNDERLINE : "";
}

const chr_t * color_blue_underline(void) {
	return color_supported() ? MAGMA_COLOR_BLUE_UNDERLINE : "";
}

const chr_t * color_purple_underline(void) {
	return color_supported() ? MAGMA_COLOR_PURPLE_UNDERLINE : "";
}

const chr_t * color_cyan_underline(void) {
	return color_supported() ? MAGMA_COLOR_CYAN_UNDERLINE : "";
}

const chr_t * color_white_underline(void) {
	return color_supported() ? MAGMA_COLOR_WHITE_UNDERLINE : "";
}

const chr_t * color_red_intense(void) {
	return color_supported() ? MAGMA_COLOR_RED_INTENSE : "";
}

const chr_t * color_green_intense(void) {
	return color_supported() ? MAGMA_COLOR_GREEN_INTENSE : "";
}

const chr_t * color_yellow_intense(void) {
	return color_supported() ? MAGMA_COLOR_YELLOW_INTENSE : "";
}

const chr_t * color_blue_intense(void) {
	return color_supported() ? MAGMA_COLOR_BLUE_INTENSE : "";
}

const chr_t * color_purple_intense(void) {
	return color_supported() ? MAGMA_COLOR_PURPLE_INTENSE : "";
}

const chr_t * color_cyan_intense(void) {
	return color_supported() ? MAGMA_COLOR_CYAN_INTENSE : "";
}

const chr_t * color_white_intense(void) {
	return color_supported() ? MAGMA_COLOR_WHITE_INTENSE : "";
}

const chr_t * color_red_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_RED_INTENSE_BOLD : "";
}

const chr_t * color_green_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_GREEN_INTENSE_BOLD : "";
}

const chr_t * color_yellow_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_YELLOW_INTENSE_BOLD : "";
}

const chr_t * color_blue_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_BLUE_INTENSE_BOLD : "";
}

const chr_t * color_purple_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_PURPLE_INTENSE_BOLD : "";
}

const chr_t * color_cyan_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_CYAN_INTENSE_BOLD : "";
}

const chr_t * color_white_intense_bold(void) {
	return color_supported() ? MAGMA_COLOR_WHITE_INTENSE_BOLD : "";
}

