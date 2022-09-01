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
/*
 * Notes: Using the first line to store the english characters.
 */
const int height = (firstByteEnd - firstByteStart+1) * cheight; 
const int width = (secondByteEnd - secondByteStart) * cwidth;

/* Using unsigned int (4 bytes) to store the RGBA */
unsigned int texture[height][width];

/* When the bitmap set to 0, we draw the backgroud, or the foreground */
unsigned int background_color = 0x00000000;
unsigned int foreground_color = 0x00FFFFFF;

struct Point {
    float x;
    float y;
};

typedef struct {
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

/*
 * A class that converts a character from GBK to Unicode.
 */
class UnicodeGBKConv
{
public:
    UnicodeGBKConv()
    {
        this -> cd = iconv_open("UNICODE", "GBK"); // from GBK to unicode
        this -> unicodeChar = 0;
    }

    size_t conv(char gbkChar[2])
    {
        char *inp = gbkChar;
        char *outp = (char *)&(this -> unicodeChar);
        this -> inbytesLeft = 2;
        this -> outbytesLeft = sizeof(this -> unicodeChar);

        return iconv(this -> cd, &inp, &(this -> inbytesLeft), &outp, &(this -> outbytesLeft));
    }

    FT_UInt32 result()
    {
        return this -> unicodeChar;
    }
private:
    iconv_t cd; /* Conversion Descriptor */
    FT_UInt32 unicodeChar;
    size_t inbytesLeft;
    size_t outbytesLeft;
};

class GlyphTexture
{
public:
    GlyphTexture(const char *fontName)
    {
        FT_Error error;

        // init freetype
        error = FT_Init_FreeType( &(this -> lib) );
        if ( error != FT_Err_Ok )
            throw "FT_Init_FreeType failed";

        error = FT_New_Face( this -> lib, fontName, 0, &(this -> face) );
        if ( error == FT_Err_Unknown_File_Format ) {
            throw "Unknown font file format";
        } else if ( error ) {
            throw "Failed to open font file";
        }

        // Specify to use the unicode
        error = FT_Select_Charmap(this -> face, ft_encoding_unicode);
        if ( error )
            throw "Failed to select charmap unicode";

        error = FT_Set_Pixel_Sizes(this -> face, (cwidth), (cheight));
        if ( error )
            throw "Failed to set face";
    }

    FT_UInt getGlyphIndex(FT_UInt32 unicodeChar)
    {
        FT_UInt glyphIndex;

        unicodeChar = unicodeChar >> 16;

        glyphIndex = FT_Get_Char_Index(this -> face, unicodeChar);
        return glyphIndex;
    }

    FT_Bitmap bitmap(FT_UInt glyphIndex)
    {
        FT_Glyph glyph;
        FT_Error error;
        // Std::cout << "Unicode Char " << unicodeChar << std::endl;

        // load the glyph image into the slot
        error = FT_Load_Glyph( this -> face, glyphIndex, FT_LOAD_DEFAULT );
        if ( error )
            throw "Failed to load glyph";

        error = FT_Get_Glyph(this -> face -> glyph, &(glyph));
        if ( error )
            throw "Failed to get glyph";

        error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        if ( error )
            throw "Faile to convert glyph";

        FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph) glyph;
        FT_Bitmap bitmap = bitmapGlyph -> bitmap;

        return bitmap;
    }
private:
    FT_Library lib;
    FT_Face face;
};

GlyphInfo getGlyphInfo(int charIndexRow, int charIndexCol)
{
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

    return ginfo;

}

void fillTexture(UnicodeGBKConv &conv, GlyphTexture &gt,
                 int charIndexRow, int charIndexCol, char gbkChar[2])
{
    auto res = conv.conv(gbkChar);

    #ifdef FILLIN_NONEXSITS_GLYPH
    FT_UInt32 index;
    if(res) {
        index = 5000;
    } else {
        auto unicodeChar = conv.result();
        index = gt.getGlyphIndex(unicodeChar);
        if(index == 0) {
	  index = 5000;
        }
    }
    #else
    if(res) {
      return;
    } 

    auto unicodeChar = conv.result();
    auto index = gt.getGlyphIndex(unicodeChar);
    if(index == 0) {
         return;
    }
    #endif

    auto bitmap = gt.bitmap(index);
    for(int xx = 0; xx < bitmap.width; ++xx) {
        for(int yy = 0; yy < bitmap.rows; ++yy) {
            unsigned char r = bitmap.buffer[yy * (bitmap.width) + xx];
            unsigned int color = 0x00FFFFFF;
            unsigned char *p = (unsigned char*)(&color);

            *(p + 3) = r;
            texture[charIndexRow *cheight + yy][charIndexCol *cwidth + xx] = color;
        }
    }
    return;
}

/*
 * Mapping a half width english ascii character to 
 * the full width english ascii character.
 */
void dbc_to_sbc(char c, char gbkChar[2])
{
    if((c & 0xff) == 0x20) {
        gbkChar[0] = 0xA1;
        gbkChar[1] = 0xA1;
    } else if((c & 0xff) >= 0x21 && (c & 0xff) <= 0x7E) {
        gbkChar[0] = 0xA3;
        gbkChar[1] = c + 0x80;
    }
}

int main(int argc, char **argv)
{
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

    GlyphTexture gt(fontName);

    for(int i = 0; i < 256; i++) {
        GlyphInfo ginfo;
        fwrite(&ginfo, sizeof(ginfo), 1, fntFp);
    }

    for(int i = 0; i < firstByteStart; i++) {
        int charIndexRow = 0;
        int charIndexCol = i;

        char c = static_cast<char>(i);

        char gbkChar[2];
        dbc_to_sbc(c, gbkChar);

        UnicodeGBKConv conv;
        fillTexture(conv, gt, charIndexRow, charIndexCol, gbkChar);
    }

    #ifdef FILLIN_NONEXSITS_GLYPH
    for(int i = firstByteStart; i < (secondByteEnd - secondByteStart); i++) {
        int charIndexRow = 0;
        int charIndexCol = i;

        char c = static_cast<char>(i);

        char gbkChar[2];
        dbc_to_sbc(c, gbkChar);

        UnicodeGBKConv conv;
        fillTexture(conv, gt, charIndexRow, charIndexCol, gbkChar);
    }
    #endif

    for(int i = firstByteStart; i < firstByteEnd; i++) {
        for(int j = secondByteStart; j < secondByteEnd; j++) {
            int charIndexRow = i-firstByteStart+1;
            int charIndexCol = j-secondByteStart;
            char gbkChar[2] = {static_cast<char>(i), static_cast<char>(j)};
            UnicodeGBKConv conv;
            fillTexture(conv, gt, charIndexRow, charIndexCol, gbkChar);
        }
    }

    auto texFp = fopen(texFile, "wb");
    fwrite(&width, sizeof(width), 1, texFp);
    fwrite(&height, sizeof(height), 1, texFp);

#ifdef TES3CN_OUTPUT	
    fwrite(&texture[0][0], sizeof(int), height*width, texFp);
#else
    for(int i = height-1; i >= 0; i--) {
        for(int j = 0; j < width; j++) {
            fwrite(&texture[i][j], sizeof(int), 1, texFp);
        }
    }
#endif

    fclose(texFp);
    fclose(fntFp);
}
