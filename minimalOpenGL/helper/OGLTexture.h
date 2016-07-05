////////////////////////////////////////////////////////////
//                                                        //
//  Simple OpenGL Texture Loader                          //
//  (w)(c)2007 Carsten Dachsbacher                        //
//                                                        //
////////////////////////////////////////////////////////////
#ifndef __TEXTURE_H
#define __TEXTURE_H
#include "GL/glew.h"
#include <GLFW/glfw3.h>

class OGLTexture
{
private:
	void	deleteTexture();
	bool	createTexture();

	GLenum	target;
	int		width, height;
	GLuint	ID;


public:
	OGLTexture( bool _rectangular = false );
	~OGLTexture();

	GLuint	getID()      { return ID-1; };
	int		getWidth()   { return width; };
	int		getHeight()  { return height; };
	
	bool	loadHDR_RGBE ( char *filename );
	bool	loadHDR_FLOAT( char *filename );
	bool	loadTGA		 ( char *fileName );
	bool	load		 ( char *filename );

	void	bind();
};

#endif
