
/**
 * @file /magma/web/register/captcha.c
 *
 * @brief	The captcha interface for the registration process.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// LOW: We shouldn't have to actually scan the fonts directory to find a valid file. Instead we could cache a list of valid fonts and then pick from it randomly.
/**
 * @brief	Select a random truetype font from the directory specified in magma.http.fonts.
 * @return	NULL on failure, or a managed string containing the pathname of the randomly selected font file on success.
 */
stringer_t * register_captcha_random_font(void) {

	DIR *directory;
	stringer_t *path;
	struct dirent64 *dp;
	size_t count = 0, selection;

	// Open the current working directory.
	if (!(directory = opendir(magma.http.fonts))) {
		log_pedantic("Unable to open the font directory. { directory = %s }", magma.http.fonts);
		return NULL;
	}

	// Count the number of fonts.
	while ((dp = readdir64(directory))) {

		if (!st_cmp_ci_ends(NULLER(dp->d_name), PLACER(".ttf", 4))) {
			count++;
		}

	};

	// No fonts were found.
	if (!count) {
		log_pedantic("The web fonts directory is empty. { directory = %s }", magma.http.fonts);
		closedir(directory);
		return NULL;
	}

	// Pick a random font.
	selection = (rand_get_uint32() % count) + 1;

	// Reset the directory stream.
	rewinddir(directory);

	// Do the loop again.
	while (selection && (dp = readdir64(directory))) {

		if (!st_cmp_ci_ends(NULLER(dp->d_name), PLACER(".ttf", 4))) {
			selection--;
		}

	}

	// Build the path.
	if (selection || !dp || !(path = st_aprint("%s/%s", magma.http.fonts, dp->d_name))) {
		log_pedantic("Could not build the font file path.");
		closedir(directory);
		return NULL;
	}

	closedir(directory);
	return path;
}

/**
 * @brief	Fill an image's background partially with pixelated noise to make it more difficult to read.
 * @param	image	a pointer to the gd image to be modified.
 * @param	x	the height, in pixels, of the image region to be filled.
 * @param	y	the width, in pixels, of the image region to be filled.
 * @return	This function returns no value.
 */
void register_captcha_write_noise(gdImagePtr image, int_t x, int_t y) {

	int_t increment, color, xpos, ypos, pixels = (x * y) * 0.05;

	// Write garbage into about 40% of the pixels.
	for (increment = 0; increment < pixels; increment++) {
		xpos = rand_get_uint32() % x;
		ypos = rand_get_uint32() % y;
		color = gdImageColorResolve_d(image, rand_get_uint32() % 256, rand_get_uint32() % 256, rand_get_uint32() % 256);
		gdImageSetPixel_d(image, xpos + 0, ypos + 0, color);
		gdImageSetPixel_d(image, xpos + 1, ypos + 0, color);
		gdImageSetPixel_d(image, xpos + 2, ypos + 0, color);
		gdImageSetPixel_d(image, xpos + 0, ypos + 1, color);
		gdImageSetPixel_d(image, xpos + 1, ypos + 1, color);
		gdImageSetPixel_d(image, xpos + 2, ypos + 2, color);
		gdImageSetPixel_d(image, xpos + 0, ypos + 2, color);
		gdImageSetPixel_d(image, xpos + 1, ypos + 2, color);
		gdImageSetPixel_d(image, xpos + 2, ypos + 2, color);
	}

	return;

}

/**
 * @brief	Generate a captcha image for a specified character string.
 * @param	value	a managed string containing the text that is to become the basis of the captcha challenge.
 * @return	NULL on failure, or a managed string containing the path to the image file containing the captcha graphic on success.
 */
stringer_t * register_captcha_generate(stringer_t *value) {

	gdImagePtr image;
	chr_t string[2], *holder = NULL;
	double font_size = 40.0, angle;
	stringer_t *output = NULL, *font_path = NULL;
	chr_t *gderr;
	int_t brect[8], characters, color, white, increment;

	if (!(characters = st_length_get(value))) {
		log_pedantic("Zero length value passed in.");
		return NULL;
	}

	// We need a font when initializing the image.
	if (!(font_path = register_captcha_random_font())) {
		log_pedantic("Could not pick a random font.");
		return NULL;
	}

	mm_wipe(string, 2);

	if ((gderr = gdImageStringFT_d(NULL, &brect[0], 0, st_char_get(font_path), font_size, 0.0, 0, 0, string))) {
		log_pedantic("Could not initialize the rectangle: %s", gderr);
		return NULL;
	}

	// Creates an image that is 36 pixels wide for each character (+24 for the margin), and 47 pixels high.
	if (!(image = gdImageCreate_d((characters * 36) + 11, 47))) {
		log_pedantic("Could not create the image.");
		return NULL;
	}

	// The first color you allocate is used for the background.
	white = gdImageColorResolve_d(image, 255, 255, 255);

	// Write a bunch of randomly colored pixels onto the background.
	register_captcha_write_noise(image, (characters * 36) + 11, 47);

	mm_wipe(string, 2);

	// Write the string.
	for (increment = 0; increment < characters; increment++) {

		// Randomize the font size. By default it goes between 22px and 38px.
		font_size = 38.0 - (rand_get_uint32() % 16);

		// Pick a random angle.
		angle = 0.0;
		if ((rand_get_uint32() % 2) == 0) {
			angle -= rand_get_uint32() % 15;
		}
		else {
			angle += rand_get_uint32() % 25;
		}

		// Change the angle to radians.
		angle /= 360.0;
		angle *= (22.000/7.000);

		// Use the appropriate character.
		string[0] = *(st_char_get(value) + increment);

		// The character i needs to always be large, so people can see the upper dot.
		if (string[0] == 'i') {
			font_size = 38;
		}

		// Select a random font.
		if (!(font_path = register_captcha_random_font())) {
			gdImageDestroy_d(image);
			log_pedantic("Could not pick a random font.");
			return NULL;
		}

		// Select a random color, biased towards red.
		color = gdImageColorResolve_d(image, rand_get_uint32() % 256, rand_get_uint32() % 50, rand_get_uint32() % 50);

		// Write the character to the image.
		if (gdImageStringFT_d(image, &brect[0], color, st_char_get(font_path), font_size, angle, (36 * increment) + 12, 43 - ((40 - font_size) /2), string) != NULL) {
			gdImageDestroy_d(image);
			log_pedantic("Could not writer characters into the image.");
			return NULL;
		}
	}

	// Output Use gdImageJpegPtr_d to produce the output in JPEG.
	if (!(holder = gdImageGifPtr_d(image, &increment)) || !increment) {
		log_pedantic("Could not output the image to a buffer.");
	}
	else if (!(output = st_import(holder, increment))) {
		log_pedantic("Could not move the image into the output buffer.");
		gdFree_d(holder);
	}
	else {
		gdFree_d(holder);
	}

	gdImageDestroy_d(image);

	return output;
}
