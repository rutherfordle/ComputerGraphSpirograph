/*
 * ImageUtils.hpp
 *
 *  Created on: Jan 3, 2012
 *      Author: Nathaniel Cesario
 */

#ifndef IMAGEUTILS_HPP_
#define IMAGEUTILS_HPP_

#include <GL/gl.h>

#include <iostream>

class ImageUByte {
public:
	ImageUByte(){
		data = NULL;
		width = -1;
		height = -1;
	}

	ImageUByte(const ImageUByte &img) {
		(*this) = img;
	}

	ImageUByte &operator=(const ImageUByte &img) {
		data = img.data;
		width = img.width;
		height = img.height;
		return (*this);
	}

	unsigned char *data;
	int width, height;
};

class TextureGL {
public:
    virtual ~TextureGL(){}

    virtual void loadFromFile(std::string file) = 0;

    /**
     * Must be called AFTER loadFromFile is called.
     *
     * Override this if you are using something other than GL_TEXTURE_2D
     */
    virtual void init();

    GLuint texName;
    float *texCoords;

    GLenum target;
    GLint level;
    GLint internalFormat;
    GLsizei width;
    GLsizei height;
    GLint border;
    GLenum format;
    GLenum type;
    GLvoid *data;
};

class UBTextureGL : public TextureGL {
public:
    UBTextureGL();
    virtual void loadFromFile(std::string file);
};

///
///Reads an image file. Currently only JPGs are supported.
///
ImageUByte readImage(std::string file);

#endif /* IMAGEUTILS_HPP_ */
