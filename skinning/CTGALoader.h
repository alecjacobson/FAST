/*
   Class Name:

      CTGALoader.

      This class loads a tga texture into this object.
*/


#ifndef CTGALOADER_H
#define CTGALOADER_H

// Remember to place all your GL/system header files that you will need to run class function

class CTGALoader
{
   public:
      CTGALoader();                       // Constructor.
      ~CTGALoader();                      // Destructor.

      bool LoadTGAFile(char *filename);   // Load a .tga image file.
      void SaveTGAScreenShot(const char *filename,
                             int width,
                             int height); // Save a .tga screen shot.
      void FreeImage();                   // Delete a image.

      unsigned int ID;                    // ID used for generating the textures in OpenGl.
      int imageWidth;                     // Width of a texture.
      int imageHeight;                    // Height of a texture.

   protected:
      void GenerateTexture();             // Generate a texture in OpenGL.
      bool LoadTGA(char *filename);       // Load a tga image.
      bool WriteTGA(const char *file, short int width,
                   short int height,
                   unsigned char *image); // Write a tga file.

      unsigned char *image;               // Texture image.
      bool textureExist;                  // This will be used if the image was loaded.
      int type;                           // Image format.
};




/*
   Class Name:

      CTGALoader.

*/


CTGALoader::CTGALoader()
{
   // Give everything default values.
  image = 0;
  textureExist = false;
  type = 0;
}


CTGALoader::~CTGALoader()
{
   FreeImage();  // Delete all images and dynamic memory.
}


bool CTGALoader::LoadTGAFile(char *file)
{
   if(textureExist)
      FreeImage();

   if(!LoadTGA(file))
      return false;

   // Make sure the image loaded.
	if(image == 0)
	   {
         return false;
	   }

   GenerateTexture();

   textureExist = true;

   return true;
}


void CTGALoader::GenerateTexture()
{
   // Generate the texture and text the id to the images id.
	glGenTextures(1, &ID);
   
   // Here we bind the texture and set up the filtering.
   glBindTexture(GL_TEXTURE_2D, ID);
   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	gluBuild2DMipmaps(GL_TEXTURE_2D, type, imageWidth, imageHeight,
                     type, GL_UNSIGNED_BYTE, image);
   glTexImage2D(GL_TEXTURE_2D, 0, type, imageWidth, imageHeight, 
                0, type, GL_UNSIGNED_BYTE, image);
}


bool CTGALoader::LoadTGA(char* file)
{
   FILE *pfile;
   unsigned char tempColor;   // This will change the images from BGR to RGB.
	unsigned char uselessChar; // This will be used to hold char data we dont want.
	short int	  uselessInt;	// This will be used to hold int data we dont want.
   int colorMode;             // If this is 3 then its in RGB, 4 is RGBA.
	long tgaSize;		         // Image size.
	long index;                // Used in the for loop.
   unsigned char imageTypeCode;
   unsigned char bitCount;
	

	// Open the image and read in binary mode.
	pfile = fopen(file, "rb");

   // check if the file opened.
	if (!pfile)
		return false;

	// Read in the two useless values.
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);
	
	// Read the image type, 2 is color, 4 is is greyscale.
	fread(&imageTypeCode, sizeof(unsigned char), 1, pfile);

	// We only want to be able to read in color or greyscale .tga's.
	if ((imageTypeCode != 2) && (imageTypeCode != 3))
	   {
		   fclose(pfile);
		   return false;
	   }


	// Get rid of 13 bytes of useless data.
	fread(&uselessInt, sizeof(short int), 1, pfile);
	fread(&uselessInt, sizeof(short int), 1, pfile);
	fread(&uselessChar, sizeof(unsigned char), 1, pfile);
	fread(&uselessInt, sizeof(short int), 1, pfile);
	fread(&uselessInt, sizeof(short int), 1, pfile);


	// Get the image width and height.
	fread(&imageWidth, sizeof(short int), 1, pfile);
	fread(&imageHeight, sizeof(short int), 1, pfile);

	// Get the bit count.
	fread(&bitCount, sizeof(unsigned char), 1, pfile);

	// Get rid of 1 byte of useless data.
   fread(&uselessChar, sizeof(unsigned char), 1, pfile);

	// If the image is RGB then colorMode should be 3 and RGBA would
   // make colorMode equal to 4.  This will help in our loop when
   // we must swap the BGR(A) to RGB(A).
	colorMode = bitCount / 8;

   // Determine the size of the tga image.
	tgaSize = imageWidth * imageHeight * colorMode;

	// Allocate memory for the tga image.
	image = (unsigned char*)malloc(sizeof(unsigned char)*tgaSize);

	// Read the image into imageData.
	fread(image, sizeof(unsigned char), tgaSize, pfile);
	
	// This loop will swap the BGR(A) to RGB(A).
	for (index = 0; index < tgaSize; index += colorMode)
	   {
		   tempColor = image[index];
		   image[index] = image[index + 2];
		   image[index + 2] = tempColor;
	   }

	// Close the file where your done.
	fclose(pfile);

   if(bitCount == 32)
      type = GL_RGBA;
   else
      type = GL_RGB;

   // return true to satisfy our if statement (load successful).
	return true;
}


void CTGALoader::FreeImage()
{
   // When the application is done delete all dynamically allocated memory.
   if(image)
      {
         free(image);
         image = 0;
         textureExist = false;
         type = 0;
      }
}


bool CTGALoader::WriteTGA(const char *file, short int width, short int height, unsigned char *outImage)
{
   // To save a screen shot is just like reading in a image.  All you do
   // is the opposite.  Istead of calling fread to read in data you call
   // fwrite to save it.

   FILE *pFile;               // The file pointer.
   unsigned char uselessChar; // used for useless char.
   short int uselessInt;      // used for useless int.
   unsigned char imageType;   // Type of image we are saving.
   int index;                 // used with the for loop.
   unsigned char bits;    // Bit depth.
   long Size;                 // Size of the picture.
   int colorMode;
   unsigned char tempColors;

   // Open file for output.
   pFile = fopen(file, "wb");

   // Check if the file opened or not.
   if(!pFile) { fclose(pFile); return false; }

   // Set the image type, the color mode, and the bit depth.
   imageType = 2; colorMode = 3; bits = 24;

   // Set these two to 0.
   uselessChar = 0; uselessInt = 0;

   // Write useless data.
   fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
   fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

   // Now image type.
   fwrite(&imageType, sizeof(unsigned char), 1, pFile);

   // Write useless data.
   fwrite(&uselessInt, sizeof(short int), 1, pFile);
   fwrite(&uselessInt, sizeof(short int), 1, pFile);
   fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);
   fwrite(&uselessInt, sizeof(short int), 1, pFile);
   fwrite(&uselessInt, sizeof(short int), 1, pFile);

   // Write the size that you want.
   fwrite(&width, sizeof(short int), 1, pFile);
   fwrite(&height, sizeof(short int), 1, pFile);
   fwrite(&bits, sizeof(unsigned char), 1, pFile);

   // Write useless data.
   fwrite(&uselessChar, sizeof(unsigned char), 1, pFile);

   // Get image size.
   Size = width * height * colorMode;

   // Now switch image from RGB to BGR.
   for(index = 0; index < Size; index += colorMode)
      {
         tempColors = outImage[index];
         outImage[index] = outImage[index + 2];
         outImage[index + 2] = tempColors;
      }

   // Finally write the image.
   fwrite(outImage, sizeof(unsigned char), Size, pFile);

   // close the file.
   fclose(pFile);

   // Make sure the writing has finished.
   pFile = fopen(file, "rb");
   fclose(pFile); 

   return true;
}


// This will save a screen shot to a file.
void CTGALoader::SaveTGAScreenShot(const char *filename, int w, int h)
{
   unsigned char *outputImage = 0;

   // Allocate the neccessary memory.
   outputImage = (unsigned char*)malloc(w * h * 3);

   // Clear the variable.
   memset(outputImage, 0, w * h * 3);

   // You use the glReadPixels() to read every pixel on the screen
   // that you specify.  You must use one less than each size.
   glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, outputImage);

   // Call our WriteTGA() function to write the tga.
   WriteTGA(filename, w, h, (unsigned char*)outputImage);

   // Clear the allocated memory.
   free(outputImage);
}

#endif