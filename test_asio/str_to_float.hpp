#ifndef TEST_ASIO_STR_TO_FLOAT_H_
#define TEST_ASIO_STR_TO_FLOAT_H_

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////

namespace irr
{
namespace core
{

const float fast_atof_table[] =	{
										0.f,
										0.1f,
										0.01f,
										0.001f,
										0.0001f,
										0.00001f,
										0.000001f,
										0.0000001f,
										0.00000001f,
										0.000000001f,
										0.0000000001f,
										0.00000000001f,
										0.000000000001f,
										0.0000000000001f,
										0.00000000000001f,
										0.000000000000001f
									};

//! Provides a fast function for converting a string into a float,
//! about 6 times faster than atof in win32.
// If you find any bugs, please send them to me, niko (at) irrlicht3d.org.
inline const char* fast_atof_move( const char* c, float& out)
{
	bool inv = false;
	char *t;
	float f;

	if (*c=='-')
	{
		c++;
		inv = true;
	}

	f = (float)strtol(c, &t, 10);

	c = t;

	if (*c == '.')
	{
		c++;

		float pl = (float)strtol(c, &t, 10);
		pl *= fast_atof_table[t-c];

		f += pl;

		c = t;

		if (*c == 'e')
		{
			++c;
			float exp = (float)strtol(c, &t, 10);
			f *= (float)pow(10.0f, exp);
			c = t;
		}
	}

	if (inv)
		f *= -1.0f;
	
	out = f;
	return c;
}

//! Provides a fast function for converting a string into a float,
//! about 6 times faster than atof in win32.
// If you find any bugs, please send them to me, niko (at) irrlicht3d.org.
inline const char* fast_atof_move_const(const char* c, float& out)
{
	bool inv = false;
	char *t;
	float f;

	if (*c=='-')
	{
		c++;
		inv = true;
	}

	f = (float)strtol(c, &t, 10);

	c = t;

	if (*c == '.')
	{
		c++;

		float pl = (float)_strtoui64(c, &t, 10);
		pl *= fast_atof_table[t-c];

		f += pl;

		c = t;

		if (*c == 'e') 
		{ 
			++c; 
			float exp = (float)strtol(c, &t, 10); 
			f *= (float)powf(10.0f, exp); 
			c = t; 
		}
	}

	if (inv)
		f *= -1.0f;
	
	out = f;
	return c;
}


inline float fast_atof(const char* c)
{
	float ret;
	fast_atof_move_const(c, ret);
	return ret;
}

} // end namespace core
}// end namespace irr

namespace xx 
{
//#include <errno.h>
//#include <ctype.h>
//#include <math.h>
//#include <float.h>
//#include <stdlib.h>

double strtod(const char *str, char **endptr)
{
  double number;
  int exponent;
  int negative;
  char *p = (char *) str;
  double p10;
  int n;
  int num_digits;
  int num_decimals;

  // Skip leading whitespace
  while (isspace(*p)) p++;

  // Handle optional sign
  negative = 0;
  switch (*p) 
  {             
    case '-': negative = 1; // Fall through to increment position
    case '+': p++;
  }

  number = 0.;
  exponent = 0;
  num_digits = 0;
  num_decimals = 0;

  // Process string of digits
  while (isdigit(*p))
  {
    number = number * 10. + (*p - '0');
    p++;
    num_digits++;
  }

  // Process decimal part
  if (*p == '.') 
  {
    p++;

    while (isdigit(*p))
    {
      number = number * 10. + (*p - '0');
      p++;
      num_digits++;
      num_decimals++;
    }

    exponent -= num_decimals;
  }

  if (num_digits == 0)
  {
    errno = ERANGE;
    return 0.0;
  }

  // Correct for sign
  if (negative) number = -number;

  // Process an exponent string
  if (*p == 'e' || *p == 'E') 
  {
    // Handle optional sign
    negative = 0;
    switch(*++p) 
    {   
      case '-': negative = 1;   // Fall through to increment pos
      case '+': p++;
    }

    // Process string of digits
    n = 0;
    while (isdigit(*p)) 
    {   
      n = n * 10 + (*p - '0');
      p++;
    }

    if (negative) 
      exponent -= n;
    else
      exponent += n;
  }

  if (exponent < DBL_MIN_EXP  || exponent > DBL_MAX_EXP)
  {
    errno = ERANGE;
    return HUGE_VAL;
  }

  // Scale the result
  p10 = 10.;
  n = exponent;
  if (n < 0) n = -n;
  while (n) 
  {
    if (n & 1) 
    {
      if (exponent < 0)
        number /= p10;
      else
        number *= p10;
    }
    n >>= 1;
    p10 *= p10;
  }

  if (number == HUGE_VAL) errno = ERANGE;
  if (endptr) *endptr = p;

  return number;
}

double atof(const char *str)
{
  return strtod(str, NULL);
}
}


struct sscanf_
{
  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    float f;
    sscanf (str, "%f", &f);
    vec.push_back (f);
  }
};
struct atof_
{
  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    vec.push_back ((float)atof (str));
  }
};
struct strtod_
{
  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    char *x = 0;
    vec.push_back ((float)strtod (str, NULL));
  }
};
struct lexical_cast_
{
  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    vec.push_back (boost::lexical_cast <float> (str));
  }
};
struct fast_strtod_
{
  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    vec.push_back ((float)xx::atof (str));
  }
};
struct irr_fast_atof_ 
{
  static inline void string_to_float (std::vector<float> &vec, const char *str)
  {
    vec.push_back (irr::core::fast_atof (str));  
  }
};

#endif  // #ifndef TEST_ASIO_STR_TO_FLOAT_H_
