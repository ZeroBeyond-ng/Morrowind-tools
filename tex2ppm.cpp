#include <iostream>
#include <stdio.h>
#include <stdlib.h>

using namespace std;
struct Pixel {
    unsigned char a;
    unsigned char b;
    unsigned char g;
    unsigned char r;
};

int main(int argc, char **argv){

   // Load tex file
   FILE *tex_file = fopen(argv[1], "rb");
   
   int x,y,height, width;
   Pixel data, pixel;

   fread(&width, sizeof(width), 1, tex_file);
   fread(&height, sizeof(height), 1, tex_file);

   cout << "width 0x" << hex << width << " height 0x" << hex << height << endl;

   FILE *imageFile;

   imageFile=fopen(argv[2],"wb");


   fprintf(imageFile,"P3\n");           // P5 filetype
   fprintf(imageFile,"%d %d\n",width,height);   // dimensions
   fprintf(imageFile,"255\n");          // Max pixel

   for(x=0;x<height;x++){
      for(y=0;y<width;y++){
         size_t readed_byte = fread(&data, sizeof(data), 1, tex_file);
         if(readed_byte != 1) {
            pixel = {0x0, 0x0, 0x0, 0x0};
	     } else {
	        pixel = data;
	     }
         fprintf(imageFile, "%d %d %d\n", pixel.r, pixel.g, pixel.b);
      }
   }
   fclose(imageFile);
}
