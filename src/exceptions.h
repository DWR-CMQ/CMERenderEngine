#ifndef QUARKGL_EXCEPTIONS_H_
#define QUARKGL_EXCEPTIONS_H_

#include <glad/glad.h>

#include <stdexcept>
#include <string>

namespace Cme 
{
	class QuarkException : public std::runtime_error
	{
		using std::runtime_error::runtime_error;
	};

	class GlException : public QuarkException
	{
		using QuarkException::QuarkException;
	};

	// Returns a string representation of the given GL error.
	std::string glErrorToString(GLenum errorCode);

	// Checks for an OpenGL error and throws a GlException if one was present.
	void checkForGlError(const char* file, int line);

	#define qrkCheckForGlError() ::Cme::checkForGlError(__FILE__, __LINE__)

}  // namespace Cme

#endif
