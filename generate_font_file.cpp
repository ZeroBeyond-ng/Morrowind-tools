/*
 * Author: ZeroBeyond
 * 
 * This file is used to generate a .fnt file and .tex file for the Morrowind game.
 * To my own knowledge, .fnt file is to store the location of each individual font
 * and the .tex file is to store the texture or the glyph.
 * 
 * This program depends on the FreeType. Please install it for compiling and executing.
 * 
 */
#include <iostream>
#include <iconv.h> // for convert to the Unicode
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <freetype2/ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <array>

/* Character height & width */
const int cheight = 16;
const int cwidth = 16;

/* Code set range */
const int firstByteStart  = 0x81;
const int firstByteEnd  = 0xFF;
const int secondByteStart  = 0x40;
const int secondByteEnd  = 0xFF;

/* File height & width */
const int height = (firstByteEnd - firstByteStart) * cheight;
const int width = (secondByteEnd - secondByteStart) * cwidth;

/* Using unsigned int (4 bytes) to store the RGBA */
unsigned int texture[height][width];

/* When the bitmap set to 0, we draw the backgroud, or the foreground */
unsigned int background_color = 0x00FFFFFF;
unsigned int foreground_color = 0x00FFFFFF;

struct Point
{
    float x;
    float y;
};

typedef struct
{
    float u1; // appears unused, always 0
    Point top_left;
    Point top_right;
    Point bottom_left;
    Point bottom_right;
    float width;
    float height;
    float u2; // appears unused, always 0
    float kerning;
    float ascent;
} GlyphInfo;

int main(int argc, char **argv) {
    char *fontName = argv[1];
    char *fntFile = argv[2];
    char *texFile = argv[3];

    /* Generate the file */
    float fontSize = (firstByteEnd - firstByteStart) * (secondByteEnd - secondByteStart);
    char fntFontName[284] = {0};

    // std::cout << fontSize << std::endl;
    sprintf(fntFontName, "%s", texFile);

    auto fntFp = fopen(fntFile, "wb");
    fwrite(&fontSize, sizeof(fontSize), 1, fntFp);

    int one = 1;
    fwrite(&one, sizeof(one), 1, fntFp);
    fwrite(&one, sizeof(one), 1, fntFp);

    fwrite(fntFontName, sizeof(fntFontName), 1, fntFp);

    FT_Library lib;
    FT_Error error;
    FT_Face face;
    FT_UInt glyphIndex;
    FT_Glyph glyph;

    // init freetype
    error = FT_Init_FreeType( &lib );

    if ( error != FT_Err_Ok )
    {
        std::cout << "BitmapFontGenerator > ERROR: FT_Init_FreeType failed, error code: " << error << std::endl;
        return false;
    }

    error = FT_New_Face( lib , fontName , 0 , &face );
    if ( error == FT_Err_Unknown_File_Format ) {
        std::cout << "BitmapFontGenerator > ERROR: failed to open file \"" 
                  << fontName << "\", unknown file format" << std::endl;
        return false;
    } else if ( error ) {
        std::cout << "BitmapFontGenerator > ERROR: failed to open file \"" 
                  << fontName << "\", error code: " << error << std::endl;
        return false;
    }

    // Specify to use the unicode
    error = FT_Select_Charmap(face, ft_encoding_unicode);
    if ( error )
    {
        std::cout << "BitmapFontGenerator > failed to select charmap: " << error << std::endl;
    }

    error = FT_Set_Pixel_Sizes(face, (cwidth), (cheight));
    if ( error )
    {
        std::cout << "BitmapFontGenerator > failed to set face: " << error << std::endl;
    }

    int count = 0;

    for(int i = firstByteStart; i < firstByteEnd; i++) {
        // std::cout << count << std::endl;

        for(int j = secondByteStart; j < secondByteEnd; j++) {
            count++;

            int charIndexRow = i - firstByteStart;
            int charIndexCol = j - secondByteStart;

            GlyphInfo ginfo;
            ginfo.u1 = 0;
            ginfo.u2 = 0;
            ginfo.top_left = {charIndexRow * cheight * 1.0 / height, charIndexCol * cwidth * 1.0 / width};
            ginfo.top_right = {charIndexRow * cheight * 1.0 / height, (charIndexCol + 1) * cwidth * 1.0 / width};
            ginfo.bottom_left = {(charIndexRow + 1) * cheight * 1.0 / height, charIndexCol * cwidth * 1.0 / width};
            ginfo.bottom_right = {(charIndexRow + 1) * cheight * 1.0 / height, (charIndexCol + 1) * cwidth * 1.0 / width} ;
            ginfo.width = cwidth;
            ginfo.height = cheight;
            ginfo.kerning = 0;
            ginfo.ascent = 0;

	    if(count < 256) 
            	fwrite(&ginfo, sizeof(ginfo), 1, fntFp);

            auto cd = iconv_open("UNICODE", "GBK"); // from GBK to unicode
            char gbkChar[2] = {static_cast<char>(i), static_cast<char>(j)};
            FT_UInt32 unicodeChar = 0;
	    FT_UInt32 utf32Char = 0;

	    char *inp = gbkChar;
	    char *outp = (char *)&unicodeChar;

            size_t inbytesLeft = 2;
            size_t outbytesLeft = sizeof(unicodeChar);

            auto res = iconv(cd, &inp, &inbytesLeft, &outp, &outbytesLeft);

	    /*

	    if(!res) {
               cd = iconv_open("UTF-32", "UNICODE"); // from GBK to unicode

	       inp = (char *)&unicodeChar;
	       outp = (char *)&utf32Char;

               inbytesLeft = sizeof(unicodeChar);
               outbytesLeft = sizeof(utf32Char);

	       std::cout << res << ' ' << inbytesLeft << ' ' << outbytesLeft << std::endl;

               res = iconv(cd, &inp, &inbytesLeft, &outp, &outbytesLeft);

	       std::cout << "Fuck " << std::endl;
	    }
	    */
	    // std::cout << res << ' ' << inbytesLeft << ' ' << outbytesLeft << std::endl;

            if(res) {

            } else {
	        unicodeChar = unicodeChar >> 16;
		// std::cout << static_cast<wchar_t>(unicodeChar) << std::endl;
                /* Draw the glyph to bitmap */
                glyphIndex = FT_Get_Char_Index(face, unicodeChar);

		std::cout << "0x" << std::hex << static_cast<unsigned char>(gbkChar[0])<< static_cast<unsigned short>(gbkChar[1])  << " 0x" 
		          << unicodeChar << ' ' << glyphIndex << std::endl;


		if(glyphIndex == 0) {
			continue;
		} else {
			std::cout << glyphIndex << std::endl;
		}

	        // std::cout << "Unicode Char " << unicodeChar << std::endl;

                // load the glyph image into the slot
                error = FT_Load_Glyph( face , glyphIndex , FT_LOAD_DEFAULT );
                if ( error )
                {
                    std::cout << "BitmapFontGenerator > failed to load glyph, error code: " << error << std::endl;
                }

                error = FT_Get_Glyph(face -> glyph, &glyph);
                if ( error )
                {
                    std::cout << "BitmapFontGenerator > failed to get glyph, error code: " << error << std::endl;
                }

                error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
                if ( error )
                {
                    std::cout << "BitmapFontGenerator > failed to convert glyph, error code: " << error << std::endl;
                }

                FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph) glyph;
                FT_Bitmap bitmap = bitmapGlyph -> bitmap;

		std::cout << bitmap.width << ' ' << bitmap.rows << std::endl;

                for(int xx = 0; xx < bitmap.width; ++xx) {
                    for(int yy = 0; yy < bitmap.rows; ++yy) {
                        unsigned char r = bitmap.buffer[yy * (bitmap.width) + xx];
                        unsigned int color = 0xFFFFFFFF;
                        unsigned char *p = 
				(unsigned char*)(&color);
                        *(p + 3) = r;
 	   

                            texture[charIndexRow *cheight + yy][charIndexCol *cwidth + xx] = color;
                    }

		    // std::cout << "\n";
                }
		// std::cout << "\n";
            }
        }
    }

    auto texFp = fopen(texFile, "wb");
    fwrite(&width, sizeof(width), 1, texFp);
    fwrite(&height, sizeof(height), 1, texFp);
    fwrite(&texture[0][0], sizeof(int), height*width, texFp);

    fclose(texFp);
    fclose(fntFp);
}
