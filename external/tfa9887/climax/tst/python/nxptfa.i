%module nxptfa	

%{
#include "inc/climax.h"
#include "inc/tfa98xx.h"
#include "inc/Tfa9887.h"
#include "inc/Tfa9887_Registers.h"
#include "inc/lxScribo.h"
#include "inc/NXP_I2C.h"


%}

%apply (char *STRING, int LENGTH) { (void *data, int length) };
//%apply (int LENGTH, char *STRING) { (int num_bytes, unsigned char *data };

%typemap(in) ( int len, u8 *)
{
  if (!PyString_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a string");
    return NULL;
  }
  $1 = PyString_Size($input);
  $2 = PyString_AsString($input);
}
%typemap(in) (nxpTfa98xxParamsType_t, void *, int )
{
  if (!PyString_Check($input)) {
    PyErr_SetString(PyExc_ValueError,"Expected a string");
    return NULL;
  }
  $3 = PyString_Size($input);
  $2 = PyString_AsString($input);
}

// This tells SWIG to treat char ** as a special case 
%typemap(in) char ** {
  /* Check if is a list */
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *o = PyList_GetItem($input,i);
      if (PyString_Check(o))
	$1[i] = PyString_AsString(PyList_GetItem($input,i));
      else {
	PyErr_SetString(PyExc_TypeError,"list must contain strings");
	free($1);
	return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

// This cleans up the char ** array we malloc'd before the function call
%typemap(freearg) char ** {
  free((char *) $1);
}

#include "inc/climax.h"
#include "inc/tfa98xx.h"
#include "inc/Tfa9887.h"
#include "inc/Tfa9887_Registers.h"
#include "inc/lxScribo.h"
#include "inc/NXP_I2C.h"
