/*
 * ImageUtils.cpp
 *
 *  Created on: Jan 3, 2012
 *      Author: Nathaniel Cesario
 */

#include "ImageUtilsGL.hpp"

#include <cstdio>
#include <cstring>
#include <jpeglib.h>
#include <string>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
//class TextureGL

void TextureGL::init() {
    glGenTextures(1, &texName);

    if (GL_TEXTURE_2D == target) {
        glBindTexture(target, texName);
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glTexImage2D(
            target,
            0,
            internalFormat,
            width,
            height,
            border,
            format,
            type,
            data
            );
}

//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//class JPEGTextureGL

UBTextureGL::UBTextureGL() {
    target = GL_TEXTURE_2D;
    level = 0;
    internalFormat = GL_RGB;
    border = 0;
    format = GL_RGB;
    type = GL_UNSIGNED_BYTE;
}

void UBTextureGL::loadFromFile(string file) {
    ImageUByte img = readImage(file);
    width = img.width;
    height = img.height;
    data = static_cast<GLvoid *>(img.data);
}

//
////////////////////////////////////////////////////////////////////////////////

ImageUByte readJPGImage(std::string file) {
	ImageUByte img;

	//TODO need to handle local paths
	FILE *fin = fopen(file.c_str(), "rb");
	if (!fin) {
		std::cerr << "**ERROR** COLLADAMinReader::getJPGData: Couldn't open "
				<< file << " for reading"
				<< std::endl;
		return img;
	}

	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	JSAMPROW row_pointer[1];

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, fin);
	jpeg_read_header(&cinfo, TRUE);

	img.width = cinfo.image_width;
	img.height = cinfo.image_height;

	jpeg_start_decompress(&cinfo);
	img.data = new unsigned char[cinfo.output_width*cinfo.output_height*cinfo.num_components];
	row_pointer[0] = new unsigned char[cinfo.output_width*cinfo.num_components];

	int idx = 0;
	while (cinfo.output_scanline < cinfo.image_height) {
		jpeg_read_scanlines(&cinfo, row_pointer, 1);
		for (size_t i = 0; i < cinfo.image_width*cinfo.num_components; i++) {
			img.data[idx] = row_pointer[0][i];
			++idx;
		}
	}

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	delete [] row_pointer[0];
	fclose(fin);

	return img;
}

ImageUByte readImage(std::string file) {
	size_t dotLoc = file.find('.');
	ImageUByte img;
	ImageUByte (*imgReader)(std::string) = NULL;

	if (std::string::npos == dotLoc) {
		std::cerr << "**WARNING** ImageUByte::readImage: "
				  << file << " does not have an extension"
				  << std::endl;
	}

	std::string fileExt = file.substr(dotLoc+1, file.length() - dotLoc);

	//TODO add more file type readers. only JPG for now
	if ("jpg" == fileExt || "jpeg" == fileExt ||
			"JPG" == fileExt || "JPEG" == fileExt) {
		imgReader = readJPGImage;
	} else { //use jpg reader by default
		imgReader = readJPGImage;
	}

	img = imgReader(file);
	return img;
}
